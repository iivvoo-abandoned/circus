#
# Some systems have problems with backspace/delete keys.. *sigh*

bind Entry <Delete> {
   if [%W selection present] {
      %W delete sel.first sel.last
   } else {
      tkEntryBackspace %W
   }
}

bind Text <Delete> {
   if {[%W tag nextrange sel 1.0 end] != ""} {
      %W delete sel.first sel.last
   } elseif [%W compare insert != 1.0] {
      %W delete insert-1c
      %W see insert
   }
}      

#
# Keys for inserting ^B ^V ^_ and ^O for bold, inverse, underline and reset

bind Entry <Control-b> {
	%W insert insert \x02
}

bind Entry <Control-c> {
	%W insert insert \x03
}

bind Entry <Control-g> {
	%W insert insert \x07
}

bind Entry <Control-o> {
	%W insert insert \x0f
}

bind Entry <Control-v> {
	%W insert insert \x16
}

bind Entry <Shift-Control-underscore> {
	%W insert insert \x1f
}

#
# More emacs-like stuff

bind Entry <Control-u> {
    if !$tk_strictMotif {
            %W delete 0 end
    }
}

