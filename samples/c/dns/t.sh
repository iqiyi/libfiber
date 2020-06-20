#!/bin/sh

./dns "www.qiyi.com"

echo ""

./dns "m.mo.mm,aaa.aaa.aaa,abc.abc.a,www.ccc.ccc,www.263.net,www.baidu.com"

echo ""
sleep 1

./dns "m.mo.mm,aaa.aaa.aaa,abc.abc.a,www.ccc.ccc,www.263.net,www.baidu.com"
