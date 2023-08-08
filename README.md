# tabletolua
lua表转换为字符串的lua实现和c实现，c实现提供了binding。只能转换key为integer和string的表，并且值不能为function、userdata、thread类型。
经测试c binding实现比lua实现大概快4倍（仅供参考）。
# build for linux
make