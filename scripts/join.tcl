#
#
# effort for a simple auto-op list for cIRCus

#
# 20 april 97 First initial version 0.01 Ivo van der Wijk

source "scripts/server.tcl"

#
# This is your auto-op list. It contains channel - mask pairs, where channel
# can also be a '*' for all channels, and mask can contain (simple) wildcards
set oplist {
	{ * *!*ivo@*nijenrode.nl }
	{ #zuurtje *!*bld@xs?.xs4all.nl }
	{ #zuurtje *!*bas@*xs4all.nl }
}

global oplist

proc auto_op {chan who} {
#
# I'm not much of a tcl programmer... next line should extract the nickname
# from a string like nick!user@host

	set nick [ string range $who 0 [ expr [ string first "!" $who ] - 1 ] ]
  	mode $chan "+o $nick"
}

proc	init_auto_op {oplist} {
# initialize autoop

	foreach pair $oplist {
		cbind JOIN [lindex $pair 0] [lindex $pair 1] auto_op
	}
}

init_auto_op $oplist

