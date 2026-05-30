module s2

require (
	lmp v0.0.0
	liblmp v0.0.0
)

replace lmp => ../../lib/go/lmp

replace liblmp => ../../lib/go/liblmp

go 1.25.9
