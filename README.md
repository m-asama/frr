IS-IS/Flex-Algo/SRv6 対応 FRRouting
=============================

## これは何？

勉強がてら FRRouting の IS-IS に SRv6 の Flex-Algo を実装してみました。

Flex-Algo については下記のインターネットドラフトを参照。

- https://tools.ietf.org/html/draft-ietf-lsr-flex-algo-05#page-15

とりあえず Metric-Type と Calc-Type はそれぞれ 0(IGP Metric) と 0(SPF) のみ対応で、
アドミニストレーティブグループを用いたトポロジー制約に対応しています。

## 使い方

まず初めに `affinity-map` コマンドでトポロジー制約に使うカラーの定義をします。

```
frr(config)# router isis srv6
frr(config-router)# affinity-map red bit-position 1
frr(config-router)# affinity-map green bit-position 2
frr(config-router)# affinity-map blue bit-position 3
frr(config-router)# affinity-map yellow bit-position 4
```

上記の実行例のうち `red` や `green` の部分がカラー名で、
`1` や `2` の部分がそのカラーのビットマップでの位置になります。

次に `flex-algo` コマンドで Flex-Algo 定義の設定をします。

```
frr(config-router)# flex-algo 131 exclude affinity red use-fapm priority 130
frr(config-router)# flex-algo 132 exclude affinity blue use-fapm priority 130
frr(config-router)# flex-algo 133 exclude affinity green use-fapm priority 130
frr(config-router)# flex-algo 134 exclude affinity yellow use-fapm priority 130
```

上記の実行例のうち `131` や `131` の部分が Flex-Algo 定義のアルゴリズム番号です。
アルゴリズム番号には 128 から 255 までの整数が指定できます。

`exclude affinity` でその Flex-Algo 定義の経路計算で使わせたくないリンクのカラーを指定します。
カラーはカンマ区切りで複数指定することも可能です。

`exclude affinity` の他に `include-all affinity` と `include-all affinity` も指定できます。

`use-fapm` を指定することでその Flex-Algo 定義の経路計算で Flex-Algo Prefix Metric を用いるかを指定します。
Flex-Algo Prefix Metric を使用しない時はこのオプションを除きます。

`priority` でその Flex-Algo 定義のプライオリティを設定します。
Flex-Algo 定義はひとつの IS-IS エリア内のどれかひとつのルータで設定すればその情報が LSP で広告され他のルータでも参照できるようになります。
複数のルータが同じアルゴリズム番号の Flex-Algo 定義を設定していた場合、このプライオリティが大きいものが用いられます。

次に `isis affinity flex-algo` コマンドでリンクのカラーを設定します。

```
frr(config)# interface ens5
frr(config-if)# isis affinity flex-algo blue
```

上記の実行例では ens5 というインターフェースのカラーを `blue` に設定しています。
カラーはカンマ区切りで複数指定することも可能です。

次に `locator` コマンドで SRv6 ロケータの定義をします。

```
frr(config)# segment-routing-ipv6
frr(config-srv6)# locator loc1 prefix 2001:db8:1:1::/64
frr(config-srv6)# locator loc2 prefix 2001:db8:1:2::/64
frr(config-srv6)# locator loc3 prefix 2001:db8:1:3::/64
frr(config-srv6)# locator loc4 prefix 2001:db8:1:4::/64
```

上記の実行例のうち `loc1` や `loc2` の部分は SRv6 ロケータの名前です。
SRv6 ロケータの名前はこの後の Flex-Algo 定義に紐づける時に用います。

最後に `srv6-locator` コマンドで SRv6 ロケータへの経路計算に使って欲しい Flex-Algo 定義のアルゴリズム番号を設定します。

```
frr(config)# router isis srv6
frr(config-router)# srv6-locator loc1 algorithm 131
frr(config-router)# srv6-locator loc2 algorithm 132
frr(config-router)# srv6-locator loc3 algorithm 133
frr(config-router)# srv6-locator loc4 algorithm 134
```

上記の設定で SRv6 ロケータが LSP で広告され、
他のルータがその SRv6 ロケータへの経路を計算する際に指定した Flex-Algo 定義のアルゴリズムが用いられることになります。

## 動作例

以下のような構成で上記実行例の設定をした場合、

```
  +-------+  blue  +-------+
  | test1 |--------| test3 |
  +-------+        +-------+
      |                |
      | red            | yellow
      |                |
  +-------+  green +-------+
  | test2 |--------| test4 |
  +-------+        +-------+
```

test2 から見た test1 の SRv6 ロケータへの経路は以下のようになります。

```
test2(config-router)# do show ipv6 route 
Codes: K - kernel route, C - connected, S - static, R - RIPng,
       O - OSPFv3, I - IS-IS, B - BGP, N - NHRP, T - Table,
       v - VNC, V - VNC-Direct, A - Babel, D - SHARP, F - PBR,
       f - OpenFabric,
       > - selected route, * - FIB route, q - queued route, r - rejected route

...
I>* 2001:db8:1:1::/64 [115/40] via fe80::5054:ff:fe1f:9f29, ens5, 00:00:30
I>* 2001:db8:1:2::/64 [115/20] via fe80::5054:ff:fec8:26bc, ens4, 00:03:15
I>* 2001:db8:1:3::/64 [115/20] via fe80::5054:ff:fec8:26bc, ens4, 00:03:15
I>* 2001:db8:1:4::/64 [115/20] via fe80::5054:ff:fec8:26bc, ens4, 00:03:15
...
```

Flex-Algo 定義 `131` が指定された `loc1` は `red` に設定されたリンクを通ることができないため、
`red` に設定されたリンクを迂回する大回りなパスが選択されています。
それ以外の `loc2`、`loc3`、`loc4` では `red` に設定されたリンクを通るパスが選択されています。

## Flex-Algo Prefix Metric

SRv6 ロケータは IS-IS のエリアをまたいで広告されないため、
エリア境界のルータでその情報を伝達するための Flex-Algo Prefix Metric という Sub-TLV が用意されています。

`is-type` が `level-1-2` のルータで以下のコマンドを実行すると LEVEL-2 の経路を LEVEL-1 にエクスポートできます。

```
frr(config)# router isis srv6
frr(config-router)# flex-algo export-fapm from-l2-to-l1
```

逆に以下のコマンドを実行すると LEVEL-1 の経路を LEVEL-2 にエクスポートできます。

```
frr(config)# router isis srv6
frr(config-router)# flex-algo export-fapm from-l1-to-l2
```

ただし LEVEL-1 から LEVEL-2 へエクスポートする際には LEVEL-2 から LEVEL-1 へエクスポートされたものは除きます。
LEVEL-2 から LEVEL-1 へエクスポートされた Flex-Algo Prefix Metric がついた IPv6 Reachability TLV には Down ビットが付けられ、
この Down ビットが付けられたものはエクスポート対象から除外するようになっているためです。

```
frr(config-router)# do show isis database detail 
...
test4.00-00          *    329   0x0000000a  0xe3fa     890    0/0/0
...
  IPv6 Reachability: 2001:db8:2:1::/64 (Metric: 4261412864) Down
    Subtlvs:
      Flex-Algo Prefix Metric Flex-Algorithm: 131 Metric: 20
  IPv6 Reachability: 2001:db8:1:2::/64 (Metric: 4261412864) Down
    Subtlvs:
      Flex-Algo Prefix Metric Flex-Algorithm: 132 Metric: 30
```
