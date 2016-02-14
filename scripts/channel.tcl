proc joinfav {chan} {
	raw "JOIN $chan"
}

toplevel .fav
listbox .fav.list
pack .fav.list 
.fav.list insert end {#cIRCus}
.fav.list insert end {#Linux} 
.fav.list insert end {#digistad}
.fav.list insert end {#zuurtje} 
bind  .fav.list <1> { joinfav [ .fav.list get active ] }

