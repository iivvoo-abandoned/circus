#
# Basic procedures to send messages to the server

proc	msg {target message} {
	raw "PRIVMSG $target: $message"
}

proc	join {target {key ""}} {
	raw "JOIN $target key"
}

proc	mode {target modes} {
	raw "MODE $target $modes"
}
