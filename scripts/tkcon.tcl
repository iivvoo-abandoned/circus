#!/bin/sh
# \
exec wish "$0" ${1+"$@"}

#
## tkcon.tcl
## Enhanced Tk Console, part of the VerTcl system
##
## Originally based off Brent Welch's Tcl Shell Widget
## (from "Practical Programming in Tcl and Tk")
##
## Thanks especially to the following for bug reports & code ideas:
## Steven Wahl <steven@indra.com>, Jan Nijtmans <nijtmans@nici.kun.nl>
## Crimmins <markcrim@umich.edu>, Wart <wart@ugcs.caltech.edu>
##
## Copyright 1995,1996 Jeffrey Hobbs
## Initiated: Thu Aug 17 15:36:47 PDT 1995
##
## jhobbs@cs.uoregon.edu, http://www.cs.uoregon.edu/~jhobbs/
##
## source standard_disclaimer.tcl
## source beer_ware.tcl
##

if {$tcl_version>=8.0} {
  package require Tk
} elseif {[catch {package require -exact Tk [expr $tcl_version-3.4]}]} {
  return -code error \
      "TkCon requires at least the stable version of tcl7.5/tk4.1"
}

foreach pkg [info loaded {}] {
  set file [lindex $pkg 0]
  set name [lindex $pkg 1]
  set version [package require $name]
  if {[string match {} [package ifneeded $name $version]]} {
    package ifneeded $name $version "load [list $file $name]"
  }
}
catch {unset file name version}

set tkCon(WWW) [info exists embed_args]

## tkConInit - inits tkCon
# ARGS:	root	- widget pathname of the tkCon console root
#	title	- title for the console root and main (.) windows
# Calls:	tkConInitUI
# Outputs:	errors found in tkCon resource file
##
proc tkConInit {} {
  global auto_path tcl_platform env tcl_pkgPath \
      tkCon argc argv tcl_interactive

  set tcl_interactive 1

  if [info exists tkCon(name)] {
    set title $tkCon(name)
  } else {
    tkConMainInit
    set title Main
  }

  array set tkCon {
    color,blink		yellow
    color,proc		darkgreen
    color,prompt	brown
    color,stdin		black
    color,stdout	blue
    color,stderr	red

    blinktime		500
    debugPrompt		{(level \#$level) debug [history nextid] > }
    font		fixed
    history		32
    dead		{}
    library		{}
    lightbrace		1
    lightcmd		1
    autoload		{}
    maineval		{}
    nontcl		0
    rcfile		.tkconrc
    scrollypos		right
    showmultiple	1
    showmenu		1
    slaveeval		{}
    subhistory		1

    exec slave app {} appname {} apptype slave cmd {} cmdbuf {} cmdsave {}
    event 1 svnt 1 cols 80 rows 24 deadapp 0 debugging 0 histid 0
    find {} find,case 0 find,reg 0
    errorInfo	{}
    slavealias	{ tkcon }
    slaveprocs	{ alias clear dir dump lremove puts echo tclindex idebug \
	unknown tcl_unknown unalias which observe observe_var auto_execok }
    version	0.70
    release	{November 1996}
    root	.
  }

  if $tkCon(WWW) {
    set tkCon(prompt1) {[history nextid] % }
  } else {
    set tkCon(prompt1) {([file tail [pwd]]) [history nextid] % }
  }

  ## If there appear to be children of '.', then make sure we use
  ## a disassociated toplevel.
  if [string compare {} [winfo children .]] {
    set tkCon(root) .tkcon
  }

  if [string compare unix $tcl_platform(platform)] {
    array set tkCon {
      font	{Courier 12 {}}
      rcfile	tkcon.cfg
    }
  }

  if [info exists env(HOME)] {
    set tkCon(rcfile) [file join $env(HOME) $tkCon(rcfile)]
  }

  ## Handle command line arguments before sourcing resource file to
  ## find if resource file is being specified (let other args pass).
  for {set i 0} {$i < $argc} {incr i} {
    if [string match \-rcfile [lindex $argv $i]] {
      set tkCon(rcfile) [lindex $argv [incr i]]
    }
  }

  if {!$tkCon(WWW) && [file exists $tkCon(rcfile)]} {
    set code [catch [list uplevel \#0 source $tkCon(rcfile)] err]
  }

  if [info exists env(TK_CON_LIBRARY)] {
    eval lappend auto_path $env(TK_CON_LIBRARY)
  } else {
    eval lappend auto_path $tkCon(library)
  }

  if {![info exists tcl_pkgPath]} {
    set dir [file join [file dirname [info nameofexec]] lib]
    if [string comp {} [info commands @scope]] {
      set dir [file join $dir itcl]
    }
    catch {source [file join $dir pkgIndex.tcl]}
  }
  catch {tclPkgUnknown dummy-name dummy-version}

  ## Handle rest of command line arguments after sourcing resource file
  ## and slave is created, but before initializing UI or setting packages.
  set slaveargs {}
  set slavefiles {}
  set truth {^(1|yes|true|on)$}
  for {set i 0} {$i < $argc} {incr i} {
    set arg [lindex $argv $i]
    if [regexp -- {-.+} $arg] {
      set val [lindex $argv [incr i]]
      ## Handle arg based options
      switch -- $arg {
	-- - -argv	{
	  set argv [concat -- [lrange $argv $i end]]
	  set argc [llength $argv]
	  break
	}
	-main - -e - -eval	{ append tkCon(maineval) $val\n }
	-package - -load	{ lappend tkCon(autoload) $val }
	-slave	{ append tkCon(slaveeval) $val\n }
	-nontcl	{ set tkCon(nontcl) [regexp -nocase $truth $val] }
	-root	{ set tkCon(root) $val }
	-rcfile	{}
	default	{ lappend slaveargs $arg; incr i -1 }
      }
    } elseif {[file isfile $arg]} {
      lappend slavefiles $arg
    } else {
      lappend slaveargs $arg
    }
  }

  ## Create slave executable
  if [string comp {} $tkCon(exec)] {
    eval tkConInitSlave $tkCon(exec) $slaveargs
  }

  ## Attach to the slave, tkConEvalAttached will then be effective
  tkConAttach $tkCon(appname) $tkCon(apptype)
  tkConInitUI $title

  ## Autoload specified packages in slave
  set pkgs [tkConEvalSlave package names]
  foreach pkg $tkCon(autoload) {
    puts -nonewline "autoloading package \"$pkg\" ... "
    if {[lsearch -exact $pkgs $pkg]>-1} {
      if [catch {tkConEvalSlave package require $pkg} pkgerr] {
	puts stderr "error:\n$pkgerr"
      } else { puts "OK" }
    } else {
      puts stderr "error: package does not exist"
    }
  }

  ## Evaluate maineval in slave
  if {[string comp {} $tkCon(maineval)] &&
      [catch {uplevel \#0 $tkCon(maineval)} merr]} {
    puts stderr "error in eval:\n$merr"
  }

  ## Source extra command line argument files into slave executable
  foreach fn $slavefiles {
    puts -nonewline "slave sourcing \"$fn\" ... "
    if {[catch {tkConEvalSlave source $fn} fnerr]} {
      puts stderr "error:\n$fnerr"
    } else { puts "OK" }
  }

  ## Evaluate slaveeval in slave
  if {[string comp {} $tkCon(slaveeval)] &&
      [catch {interp eval $tkCon(exec) $tkCon(slaveeval)} serr]} {
    puts stderr "error in slave eval:\n$serr"
  }
  ## Output any error/output that may have been returned from rcfile
  if {[info exists code] && [string comp {} $err]} {
    if $code {
      puts stderr "error in $tkCon(rcfile):\n$err"
    } else {
      puts stdout "returned from $tkCon(rcfile):\n$err"
    }
  }
  tkConStateCheckpoint [concat $tkCon(name) $tkCon(exec)] slave
  tkConStateCheckpoint $tkCon(name) slave
}

## tkConInitSlave - inits the slave by placing key procs and aliases in it
## It's arg[cv] are based on passed in options, while argv0 is the same as
## the master.  tcl_interactive is the same as the master as well.
# ARGS:	slave	- name of slave to init.  If it does not exist, it is created.
#	args	- args to pass to a slave as argv/argc
##
proc tkConInitSlave {slave args} {
  global tkCon argv0 tcl_interactive tcl_library env
  if [string match {} $slave] {
    return -code error "Don't init the master interpreter, goofball"
  }
  if ![interp exists $slave] { interp create $slave }
  if {[interp eval $slave info command source] == ""} {
    $slave alias source tkConSafeSource $slave
    $slave alias load tkConSafeLoad $slave
    $slave alias open tkConSafeOpen $slave
    $slave alias exit exit
    $slave alias file file
    interp eval $slave [dump var tcl_library env]
    interp eval $slave [list source [file join $tcl_library init.tcl]]
  }
  interp eval $slave {catch {rename puts tcl_puts}}
  foreach cmd $tkCon(slaveprocs) { interp eval $slave [dump proc $cmd] }
  foreach cmd $tkCon(slavealias) { interp alias $slave $cmd {} $cmd }
  interp alias $slave ls $slave dir
  interp eval $slave set tcl_interactive $tcl_interactive \; \
      set argv0 [list $argv0] \; set argc [llength $args] \; \
      set argv  [list $args] \; history keep $tkCon(history) \; {
    if {[string match {} [info command bgerror]]} {
      proc bgerror err {
	global errorInfo
	set body [info body bgerror]
	rename bgerror {}
	if [auto_load bgerror] { return [bgerror $err] }
	proc bgerror err $body
	tkcon bgerror $err $errorInfo
      }
    }
  }

  foreach pkg [lremove [package names] Tcl] {
    foreach v [package versions $pkg] {
      interp eval $slave [list package ifneeded $pkg $v \
			      [package ifneeded $pkg $v]]
    }
  }
}

## tkConInitInterp - inits an interpreter by placing key
## procs and aliases in it.
# ARGS: name	- interp name
#	type	- interp type (slave|interp)
##
proc tkConInitInterp {name type} {
  global tkCon
  ## Don't allow messing up a local master interpreter
  if {[string match slave $type] && \
      [regexp {^([Mm]ain|Slave[0-9]+)$} $name]} return
  set old [tkConAttach]
  catch {
    tkConAttach $name $type
    tkConEvalAttached {catch {rename puts tcl_puts}}
    foreach cmd $tkCon(slaveprocs) { tkConEvalAttached [dump proc $cmd] }
    if [string match slave $type] {
      foreach cmd $tkCon(slavealias) {
	tkConMain interp alias $name $cmd $tkCon(name) $cmd
      }
    } else {
      set name [tk appname]
      foreach cmd $tkCon(slavealias) {
	tkConEvalAttached "proc $cmd args { send [list $name] $cmd \$args }"
      }
    }
    ## Catch in case it's a 7.4 (no 'interp alias') interp
    tkConEvalAttached {catch {interp alias {} ls {} dir}}
    return
  } err
  eval tkConAttach $old
  if [string comp {} $err] { return -code error $err }
}

## tkConInitUI - inits UI portion (console) of tkCon
## Creates all elements of the console window and sets up the text tags
# ARGS:	root	- widget pathname of the tkCon console root
#	title	- title for the console root and main (.) windows
# Calls:	tkConInitMenus, tkConPrompt
##
proc tkConInitUI {title} {
  global tkCon

  set root $tkCon(root)
  if [string match . $root] { set w {} } else { set w [toplevel $root] }
  catch {wm withdraw $root}
  set tkCon(base) $w

  ## Menus
  option add *Menu.font $tkCon(font) widgetDefault
  set tkCon(menubar) [frame $w.mbar -relief raised -bd 2]
  set tkCon(console) [set con [text $w.text -font $tkCon(font) -wrap char \
      -yscrollcommand "$w.sy set" -setgrid 1 -foreground $tkCon(color,stdin) \
      -width $tkCon(cols) -height $tkCon(rows)]]
  bindtags $con "$con PreCon Console PostCon $root all"
  set tkCon(scrolly) [scrollbar $w.sy -takefocus 0 -bd 1 -command "$con yview"]

  tkConInitMenus $tkCon(menubar) $title
  tkConBindings

  if $tkCon(showmenu) { pack $tkCon(menubar) -fill x }
  pack $w.sy -side $tkCon(scrollypos) -fill y
  pack $con -fill both -expand 1

  tkConPrompt "$title console display active\n"

  foreach col {prompt stdout stderr stdin proc} {
    $con tag configure $col -foreground $tkCon(color,$col)
  }
  $con tag configure blink -background $tkCon(color,blink)
  $con tag configure find -background $tkCon(color,blink)

  if ![catch {wm title $root "tkCon $tkCon(version) $title"}] {
    bind $con <Configure> {
      scan [wm geometry [winfo toplevel %W]] "%%dx%%d" tkCon(cols) tkCon(rows)
    }
    wm deiconify $root
  }
  focus -force $tkCon(console)
}

## tkConEval - evaluates commands input into console window
## This is the first stage of the evaluating commands in the console.
## They need to be broken up into consituent commands (by tkConCmdSep) in
## case a multiple commands were pasted in, then each is eval'ed (by
## tkConEvalCmd) in turn.  Any uncompleted command will not be eval'ed.
# ARGS:	w	- console text widget
# Calls:	tkConCmdGet, tkConCmdSep, tkConEvalCmd
## 
proc tkConEval {w} {
  global tkCon
  tkConCmdSep [tkConCmdGet $w] cmds tkCon(cmd)
  $w mark set insert end-1c
  $w insert end \n
  if [llength $cmds] {
    foreach cmd $cmds {tkConEvalCmd $w $cmd}
    $w insert insert $tkCon(cmd) {}
  } elseif {[info complete $tkCon(cmd)] && ![regexp {[^\\]\\$} $tkCon(cmd)]} {
    tkConEvalCmd $w $tkCon(cmd)
  }
  $w see insert
}

## tkConEvalCmd - evaluates a single command, adding it to history
# ARGS:	w	- console text widget
# 	cmd	- the command to evaluate
# Calls:	tkConPrompt
# Outputs:	result of command to stdout (or stderr if error occured)
# Returns:	next event number
## 
proc tkConEvalCmd {w cmd} {
  global tkCon
  $w mark set output end
  if [string comp {} $cmd] {
    set err 0
    if $tkCon(subhistory) {
      set ev [tkConEvalSlave history nextid]
      incr ev -1
      if {[string match !! $cmd]} {
	set err [catch {tkConEvalSlave history event $ev} cmd]
	if !$err {$w insert output $cmd\n stdin}
      } elseif {[regexp {^!(.+)$} $cmd dummy event]} {
	set err [catch {tkConEvalSlave history event $event} cmd]
	if !$err {$w insert output $cmd\n stdin}
      } elseif {[regexp {^\^([^^]*)\^([^^]*)\^?$} $cmd dummy old new]} {
	if ![set err [catch {tkConEvalSlave history event $ev} cmd]] {
	  regsub -all -- $old $cmd $new cmd
	  $w insert output $cmd\n stdin
	}
      }
    }
    if $err {
      $w insert output $cmd\n stderr
    } else {
      if [string match {} $tkCon(appname)] {
	if [catch {tkConEvalSlave eval $cmd} res] {
	  set tkCon(errorInfo) [tkConEvalSlave set errorInfo]
	  set err 1
	}
      } else {
	if {$tkCon(nontcl) && [string match interp $tkCon(apptype)]} {
	  if [catch "tkConEvalSend $cmd" res] {
	    set tkCon(errorInfo) {Non-Tcl errorInfo not available}
	    set err 1
	  }
	} else {
	  if [catch [list tkConEvalAttached $cmd] res] {
	    if [catch {tkConEvalAttached set errorInfo} err] {
	      set tkCon(errorInfo) {Error attempting to retrieve errorInfo}
	    } else {
	      set tkCon(errorInfo) $err
	    }
	    set err 1
	  }
	}
      }
      tkConEvalSlave history add $cmd
      if $err {
	$w insert output $res\n stderr
      } elseif {[string comp {} $res]} {
	$w insert output $res\n stdout
      }
    }
  }
  tkConPrompt
  set tkCon(event) [tkConEvalSlave history nextid]
}

## tkConEvalSlave - evaluates the args in the associated slave
## args should be passed to this procedure like they would be at
## the command line (not like to 'eval').
# ARGS:	args	- the command and args to evaluate
##
proc tkConEvalSlave args {
  global tkCon
  interp eval $tkCon(exec) $args
}

## tkConEvalOther - evaluate a command in a foreign interp or slave
## without attaching to it.  No check for existence is made.
# ARGS:	app	- interp/slave name
#	type	- (slave|interp)
##
proc tkConEvalOther {app type args} {
  if [string match slave $type] {
    if [string match Main $app] { set app {} }
    tkConMain interp eval $app $args
  } else {
    eval send [list $app] $args
  }
}

## tkConEvalSend - sends the args to the attached interpreter
## Varies from 'send' by determining whether attachment is dead
## when an error is received
# ARGS:	args	- the args to send across
# Returns:	the result of the command
##
proc tkConEvalSend args {
  global tkCon
  if $tkCon(deadapp) {
    if {[lsearch -exact [winfo interps] $tkCon(app)]<0} {
      return
    } else {
      set tkCon(appname) [string range $tkCon(appname) 5 end]
      set tkCon(deadapp) 0
      tkConPrompt "\n\"$tkCon(app)\" alive\n" [tkConCmdGet $tkCon(console)]
    }
  }
  set code [catch {eval send [list $tkCon(app)] $args} result]
  if {$code && [lsearch -exact [winfo interps] $tkCon(app)]<0} {
    ## Interpreter disappeared
    if {[string compare leave $tkCon(dead)] && \
	([string match ignore $tkCon(dead)] || \
	[tk_dialog $tkCon(base).dead "Dead Attachment" \
	"\"$tkCon(app)\" appears to have died.\
	\nReturn to primary slave interpreter?" questhead 0 OK No])} {
      set tkCon(appname) "DEAD:$tkCon(appname)"
      set tkCon(deadapp) 1
    } else {
      set err "Attached Tk interpreter \"$tkCon(app)\" died."
      tkConAttach {}
      set tkCon(deadapp) 0
      tkConEvalSlave set errorInfo $err
    }
    tkConPrompt \n [tkConCmdGet $tkCon(console)]
  }
  return -code $code $result
}

## tkConCmdGet - gets the current command from the console widget
# ARGS:	w	- console text widget
# Returns:	text which compromises current command line
## 
proc tkConCmdGet w {
  if [string match {} [set ix [$w tag nextrange prompt limit end]]] {
    $w tag add stdin limit end-1c
    return [$w get limit end-1c]
  }
}

## tkConCmdSep - separates multiple commands into a list and remainder
# ARGS:	cmd	- (possible) multiple command to separate
# 	list	- varname for the list of commands that were separated.
#	rmd	- varname of any remainder (like an incomplete final command).
#		If there is only one command, it's placed in this var.
# Returns:	constituent command info in varnames specified by list & rmd.
## 
proc tkConCmdSep {cmd ls rmd} {
  upvar $ls cmds $rmd tmp
  set tmp {}
  set cmds {}
  foreach cmd [split [set cmd] \n] {
    if [string comp {} $tmp] {
      append tmp \n$cmd
    } else {
      append tmp $cmd
    }
    if {[info complete $tmp] && ![regexp {[^\\]\\$} $tmp]} {
      lappend cmds $tmp
      set tmp {}
    }
  }
  if {[string comp {} [lindex $cmds end]] && [string match {} $tmp]} {
    set tmp [lindex $cmds end]
    set cmds [lreplace $cmds end end]
  }
}

## tkConPrompt - displays the prompt in the console widget
# ARGS:	w	- console text widget
# Outputs:	prompt (specified in tkCon(prompt1)) to console
## 
proc tkConPrompt {{pre {}} {post {}} {prompt {}}} {
  global tkCon
  set w $tkCon(console)
  if [string comp {} $pre] { $w insert end $pre stdout }
  set i [$w index end-1c]
  if [string comp {} $tkCon(appname)] {
    $w insert end ">$tkCon(appname)< " prompt
  }
  if [string comp {} $prompt] {
    $w insert end $prompt prompt
  } else {
    $w insert end [tkConEvalSlave subst $tkCon(prompt1)] prompt
  }
  $w mark set output $i
  $w mark set insert end
  $w mark set limit insert
  $w mark gravity limit left
  if [string comp {} $post] { $w insert end $post stdin }
  $w see end
}

## tkConAbout - gives about info for tkCon
## 
proc tkConAbout {} {
  global tkCon
  tk_dialog $tkCon(base).about "About TkCon v$tkCon(version)" \
      "Jeffrey Hobbs, Copyright 1995-96\njhobbs@cs.uoregon.edu\
      \nhttp://www.cs.uoregon.edu/~jhobbs/\
      \nRelease Date: v$tkCon(version), $tkCon(release)" questhead 0 OK
}

## tkConHelp - gives help info for tkCon
##
proc tkConHelp {} {
  global tkCon
  set page	"http://www.cs.uoregon.edu/~jhobbs/work/tkcon/"
  set email	"jhobbs@cs.uoregon.edu"
  if [tk_dialog $tkCon(base).help "Help on TkCon v$tkCon(version)" \
	  "Jeffrey Hobbs, $email\nHelp available at:\n$page" \
	  questhead 0 OK "Load into Netscape"] {
    update
    if {[catch {exec netscape -remote "openURL($page)"}]
	&& [catch {exec netscape $page &}]} {
      tk_dialog $tkCon(base).dialog "Couldn't exec Netscape" \
	  "Couldn't exec Netscape.\nMake sure it's in your path" \
	  warning 0 Bummer
    }
  }
}

## tkConInitMenus - inits the menubar and popup for the console
# ARGS:	w	- console text widget
## 
proc tkConInitMenus {w title} {
  global tkCon

  if [catch {menu $w.pop -tearoff 0}] {
    label $w.label -text "Menus not available in plugin mode"
    pack $w.label
    return
  }
  bind [winfo toplevel $w] <Button-3> "tk_popup $w.pop %X %Y"

  pack [menubutton $w.con  -text "Console"  -un 0 -menu $w.con.m] -side left
  $w.pop add cascade -label "Console" -un 0 -menu $w.pop.con

  pack [menubutton $w.edit -text "Edit"     -un 0 -menu $w.edit.m] -side left
  $w.pop add cascade -label "Edit"    -un 0 -menu $w.pop.edit

  pack [menubutton $w.int -text "Interp"    -un 0 -menu $w.int.m] -side left
  $w.pop add cascade -label "Interp"  -un 0 -menu $w.pop.int

  pack [menubutton $w.pref -text "Prefs"    -un 0 -menu $w.pref.m] -side left
  $w.pop add cascade -label "Prefs"   -un 0 -menu $w.pop.pref

  pack [menubutton $w.hist -text "History"  -un 0 -menu $w.hist.m] -side left
  $w.pop add cascade -label "History"   -un 0 -menu $w.pop.hist

  pack [menubutton $w.help -text "Help"     -un 0 -menu $w.help.m] -side right
  $w.pop add cascade -label "Help"    -un 0 -menu $w.pop.help

  ## Console Menu
  ##
  foreach m [list [menu $w.con.m -disabledfore $tkCon(color,prompt)] \
		 [menu $w.pop.con -disabledfore $tkCon(color,prompt)]] {
    $m add command -label "$title Console" -state disabled
    $m add command -label "New Console" -un 0 -acc Ctrl-N -com tkConNew
    $m add command -label "Close Console " -un 0 -acc Ctrl-w -com tkConDestroy
    $m add command -label "Clear Console " -un 1 -acc Ctrl-l \
	-com { clear; tkConPrompt }
    $m add separator
    $m add cascade -label "Attach Console " -un 0 -menu $m.apps
    $m add separator
    $m add command -label "Quit" -un 0 -acc Ctrl-q -command exit

    ## Attach Console Menu
    ##
    menu $m.apps -disabledforeground $tkCon(color,prompt) \
	-postcommand "tkConFillAppsMenu $m.apps"
  }

  ## Edit Menu
  ##
  set text $tkCon(console)
  foreach m [list [menu $w.edit.m] [menu $w.pop.edit]] {
    $m add command -label "Cut"   -un 1 -acc Ctrl-x -command "tkConCut $text"
    $m add command -label "Copy"  -un 1 -acc Ctrl-c -command "tkConCopy $text"
    $m add command -label "Paste" -un 0 -acc Ctrl-v -command "tkConPaste $text"
    $m add separator
    $m add command -label "Find"  -un 0 -acc Ctrl-F \
	-command "tkConFindBox $text"
  }

  ## Interp Menu
  ##
  foreach m [list $w.int.m $w.pop.int] {
    menu $m -disabledfore $tkCon(color,prompt) -postcom "tkConInterpMenu $m"
  }

  ## Prefs Menu
  ##
  foreach m [list [menu $w.pref.m] [menu $w.pop.pref]] {
    $m add checkbutton -label "Brace Highlighting"    -var tkCon(lightbrace)
    $m add checkbutton -label "Command Highlighting"  -var tkCon(lightcmd)
    $m add checkbutton -label "History Substitution"  -var tkCon(subhistory)
    $m add checkbutton -label "Non-Tcl Attachments"   -var tkCon(nontcl)
    $m add checkbutton -label "Show Multiple Matches" -var tkCon(showmultiple)
    $m add checkbutton -label "Show Menubar"	      -var tkCon(showmenu) \
	-command "if \$tkCon(showmenu) { \
	pack $w -fill x -before $tkCon(console) -before $tkCon(scrolly) \
      } else { pack forget $w }"
    $m add cascade -label Scrollbar -un 0 -menu $m.scroll

    ## Scrollbar Menu
    ##
    set m [menu $m.scroll -tearoff 0]
    $m add radio -label "Left" -var tkCon(scrollypos) -value left -command {
      pack config $tkCon(scrolly) -side left
    }
    $m add radio -label "Right" -var tkCon(scrollypos) -value right -command {
      pack config $tkCon(scrolly) -side right
    }
  }

  ## History Menu
  ##
  foreach m [list $w.hist.m $w.pop.hist] {
    menu $m -disabledfore $tkCon(color,prompt) -postcom "tkConHistoryMenu $m"
  }

  ## Help Menu
  ##
  foreach m [list [menu $w.help.m] [menu $w.pop.help]] {
    $m add command -label "About " -un 0 -acc Ctrl-A -command tkConAbout
    $m add separator
    $m add command -label "Help" -un 0 -acc Ctrl-H -command tkConHelp
  }
}

## tkConHistoryMenu - dynamically build the menu for attached interpreters
##
# ARGS:	w	- menu widget
##
proc tkConHistoryMenu w {
  global tkCon

  if ![winfo exists $w] return
  set id [tkConEvalSlave history nextid]
  if {$tkCon(histid)==$id} return
  set tkCon(histid) $id
  $w delete 0 end
  while {($id>$tkCon(histid)-10) && \
      ![catch {tkConEvalSlave history event [incr id -1]} tmp]} {
    set lbl [lindex [split $tmp "\n"] 0]
    if {[string len $lbl]>32} { set lbl [string range $tmp 0 30]... }
    $w add command -label "$id: $lbl" -command "
    $tkCon(console) delete limit end
    $tkCon(console) insert limit [list $tmp]
    $tkCon(console) see end
    tkConEval $tkCon(console)
    "
  }
}

## tkConInterpMenu - dynamically build the menu for attached interpreters
##
# ARGS:	w	- menu widget
##
proc tkConInterpMenu w {
  global tkCon

  if ![winfo exists $w] return
  set i [tkConAttach]
  set app  [lindex $i 0]
  set type [lindex $i 1]
  $w delete 0 end
  $w add command -label "[string toup $type]: $app" -state disabled
  $w add separator
  if {($tkCon(nontcl) && [string match interp $type]) || $tkCon(deadapp)} {
    $w add command -state disabled -label "Communication disabled to"
    $w add command -state disabled -label "dead or non-Tcl interps"
    return
  }

  set isnew [tkConEvalAttached expr \[info tclversion\]>7.4]
  set hastk [tkConEvalAttached info exists tk_library]

  if 0 {
  ## Inspect Cascaded Menu
  ##
  $w add cascade -label Inspect -un 0 -menu $w.ins
  set m $w.ins
  if [winfo exists $m] {
    $m delete 0 end
  } else {
    menu $m -tearoff no -disabledfore $tkCon(color,prompt)
  }
  $m add check -label "Procedures" \
      -command [list tkConInspect $app $type procs]
  $m add check -label "Global Vars" \
      -command [list tkConInspect $app $type vars]
  if $isnew {
    $m add check -label "Interpreters" \
	-command [list tkConInspect $app $type interps]
    $m add check -label "Aliases" \
	-command [list tkConInspect $app $type aliases]
  }
  if $hastk {
    $m add separator
    $m add check -label "All Widgets" \
	-command [list tkConInspect $app $type widgets]
    $m add check -label "Canvas Widgets" \
	-command [list tkConInspect $app $type canvases]
    $m add check -label "Menu Widgets" \
	-command [list tkConInspect $app $type menus]
    $m add check -label "Text Widgets" \
	-command [list tkConInspect $app $type texts]
    if $isnew {
      $m add check -label "Images" \
	  -command [list tkConInspect $app $type images]
    }
  }
  }

  if $isnew {
    ## Packages Cascaded Menu
    ##
    $w add cascade -label Packages -un 0 -menu $w.pkg
    set m $w.pkg
    if [winfo exists $m] { $m delete 0 end } else {
      menu $m -tearoff no -disabledfore $tkCon(color,prompt)
    }

    foreach pkg [tkConEvalAttached [list info loaded {}]] {
      set loaded([lindex $pkg 1]) [package provide $pkg]
    }
    foreach pkg [lremove [tkConEvalAttached package names] Tcl] {
      set version [tkConEvalAttached package provide $pkg]
      if [string comp {} $version] {
	set loaded($pkg) $version
      } elseif {![info exists loaded($pkg)]} {
	set loadable($pkg) [list package require $pkg]
      }
    }
    foreach pkg [tkConEvalAttached info loaded] {
      set pkg [lindex $pkg 1]
      if {![info exists loaded($pkg)] && ![info exists loadable($pkg)]} {
	set loadable($pkg) [list load {} $pkg]
      }
    }
    foreach pkg [array names loadable] {
      $m add command -label "Load $pkg ([tkConEvalAttached package version $pkg])" \
	  -command "tkConEvalOther [list $app] $type $loadable($pkg)"
    }
    if {[info exists loaded] && [info exists loadable]} { $m add separator }
    foreach pkg [array names loaded] {
      $m add command -label "${pkg}$loaded($pkg) Loaded" -state disabled
    }
  }

  ## Show Last Error
  ##
  $w add separator
  $w add command -label "Show Last Error" \
      -command "tkcon error [list $app] $type"

  ## State Checkpoint/Revert
  ##
  $w add separator
  $w add command -label "Checkpoint State" \
      -command [list tkConStateCheckpoint $app $type]
  $w add command -label "Revert State" \
      -command [list tkConStateRevert $app $type]
  $w add command -label "View State Change" \
      -command [list tkConStateCompare $app $type]

  ## Init Interp
  ##
  $w add separator
  $w add command -label "Send TkCon Commands" \
      -command [list tkConInitInterp $app $type]
}

## tkConFillAppsMenu - fill in  in the applications sub-menu
## with a list of all the applications that currently exist.
##
proc tkConFillAppsMenu {m} {
  global tkCon

  array set interps [set tmp [tkConInterps]]
  foreach {i j} $tmp { set tknames($j) {} }

  catch {$m delete 0 last}
  set cmd {tkConPrompt \n [tkConCmdGet $tkCon(console)]}
  $m add radio -label {None (use local slave) } -var tkCon(app) \
      -value [concat $tkCon(name) $tkCon(exec)] -acc Ctrl-1 \
      -command "tkConAttach {}; $cmd"
  $m add separator
  $m add command -label "Foreign Tk Interpreters" -state disabled
  foreach i [lsort [lremove [winfo interps] [array names tknames]]] {
    $m add radio -label $i -var tkCon(app) -value $i \
	-command "tkConAttach [list $i] interp; $cmd"
  }
  $m add separator

  $m add command -label "TkCon Interpreters" -state disabled
  foreach i [lsort [array names interps]] {
    if [string match {} $interps($i)] { set interps($i) "no Tk" }
    if [regexp {^Slave[0-9]+} $i] {
      set opts [list -label "$i ($interps($i))" -var tkCon(app) -value $i \
	  -command "tkConAttach [list $i] slave; $cmd"]
      if [string match $tkCon(name) $i] { append opts " -acc Ctrl-2" }
      eval $m add radio $opts
    } else {
      set name [concat Main $i]
      if [string match Main $name] {
	$m add radio -label "$name ($interps($i))" -var tkCon(app) \
	    -value Main -acc Ctrl-3 \
	    -command "tkConAttach [list $name] slave; $cmd"
      } else {
	$m add radio -label "$name ($interps($i))" -var tkCon(app) -value $i \
	     -command "tkConAttach [list $name] slave; $cmd"
      }
    }
  }
}

## tkConFindBox - creates minimal dialog interface to tkConFind
# ARGS:	w	- text widget
#	str	- optional seed string for tkCon(find)
##
proc tkConFindBox {w {str {}}} {
  global tkCon

  set base $tkCon(base).find
  if ![winfo exists $base] {
    toplevel $base
    wm withdraw $base
    wm title $base "TkCon Find"

    pack [frame $base.f] -fill x -expand 1
    label $base.f.l -text "Find:"
    entry $base.f.e -textvar tkCon(find)
    pack [frame $base.opt] -fill x
    checkbutton $base.opt.c -text "Case Sensitive" -variable tkCon(find,case)
    checkbutton $base.opt.r -text "Use Regexp" -variable tkCon(find,reg)
    pack $base.f.l -side left
    pack $base.f.e $base.opt.c $base.opt.r -side left -fill both -expand 1
    pack [frame $base.sep -bd 2 -relief sunken -height 4] -fill x
    pack [frame $base.btn] -fill both
    button $base.btn.fnd -text "Find" -width 6
    button $base.btn.clr -text "Clear" -width 6
    button $base.btn.dis -text "Dismiss" -width 6
    eval pack [winfo children $base.btn] -padx 4 -pady 2 -side left -fill both

    focus $base.f.e

    bind $base.f.e <Return> [list $base.btn.fnd invoke]
    bind $base.f.e <Escape> [list $base.btn.dis invoke]
  }
  $base.btn.fnd config -command "tkConFind $w \$tkCon(find) \
      -case \$tkCon(find,case) -reg \$tkCon(find,reg)"
  $base.btn.clr config -command "
  $w tag remove find 1.0 end
  set tkCon(find) {}
  "
  $base.btn.dis config -command "
  $w tag remove find 1.0 end
  wm withdraw $base
  "
  if [string comp {} $str] {
    set tkCon(find) $str
    $base.btn.fnd invoke
  }

  if {[string comp normal [wm state $base]]} {
    wm deiconify $base
  } else { raise $base }
  $base.f.e select range 0 end
}

## tkConFind - searches in text widget $w for $str and highlights it
## If $str is empty, it just deletes any highlighting
# ARGS: w	- text widget
#	str	- string to search for
#	-case	TCL_BOOLEAN	whether to be case sensitive	DEFAULT: 0
#	-regexp	TCL_BOOLEAN	whether to use $str as pattern	DEFAULT: 0
##
proc tkConFind {w str args} {
  $w tag remove find 1.0 end
  set truth {^(1|yes|true|on)$}
  set opts  {}
  foreach {key val} $args {
    switch -glob -- $key {
      -c* { if [regexp -nocase $truth $val] { set case 1 } }
      -r* { if [regexp -nocase $truth $val] { lappend opts -regexp } }
      default { return -code error "Unknown option $key" }
    }
  }
  if ![info exists case] { lappend opts -nocase }
  if [string match {} $str] return
  $w mark set findmark 1.0
  while {[string comp {} [set ix [eval $w search $opts -count numc -- \
      [list $str] findmark end]]]} {
    $w tag add find $ix ${ix}+${numc}c
    $w mark set findmark ${ix}+1c
  }
  catch {$w see find.first}
  return [expr [llength [$w tag ranges find]]/2]
}

## tkConAttach - called to attach tkCon to an interpreter
# ARGS:	an	- application name to which tkCon sends commands
#		  This is either a slave interperter name or tk appname.
#	type	- (slave|interp) type of interpreter we're attaching to
#		  slave means it's a TkCon interpreter
#		  interp means we'll need to 'send' to it.
# Results:	tkConEvalAttached is recreated to evaluate in the
#		appropriate interpreter
##
proc tkConAttach {{an <NONE>} {type slave}} {
  global tkCon
  if [string match <NONE> $an] {
    if [string match {} $tkCon(appname)] {
      return [list [concat $tkCon(name) $tkCon(exec)] $tkCon(apptype)]
    } else {
      return [list $tkCon(appname) $tkCon(apptype)]
    }
  }
  set app -
  set path [concat $tkCon(name) $tkCon(exec)]
  if [string comp {} $an] {
    array set interps [tkConInterps]
    if [string match {[Mm]ain} [lindex $an 0]] { set an [lrange $an 1 end] }
    if {[string match $path $an]} {
      set an {}
      set app $path
      set type slave
    } elseif {[info exists interps($an)]} {
      if [string match {} $an] { set an Main; set app Main }
      set type slave
    } elseif {[interp exists $an]} {
      set an [concat $tkCon(name) $an]
      set type slave
    } elseif {[interp exists [concat $tkCon(exec) $an]]} {
      set an [concat $path $an]
      set type slave
    } elseif {[lsearch [winfo interps] $an] > -1} {
      if {[tkConEvalSlave info exists tk_library]
	  && [string match $an [tkConEvalSlave tk appname]]} {
	set an {}
	set app $path
	set type slave
      } elseif {[set i [lsearch [tkConMain set tkCon(interps)] $an]] > -1} {
	set an [lindex [tkConMain set tkCon(slaves)] $i]
	if [string match {[Mm]ain} $an] { set app Main }
	set type slave
      } else {
	set type interp
      }
    } else {
      return -code error "No known interpreter \"$an\""
    }
  } else {
    set app $path
  }
  if [string match - $app] { set app $an }
  set tkCon(app)     $app
  set tkCon(appname) $an
  set tkCon(apptype) $type
  set tkCon(deadapp) 0

  ## tkConEvalAttached - evaluates the args in the attached interp
  ## args should be passed to this procedure as if they were being
  ## passed to the 'eval' procedure.  This procedure is dynamic to
  ## ensure evaluation occurs in the right interp.
  # ARGS:	args	- the command and args to evaluate
  ##
  switch $type {
    slave {
      if [string match {} $an] {
	interp alias {} tkConEvalAttached {} tkConEvalSlave eval
      } elseif {[string match Main $tkCon(app)]} {
	interp alias {} tkConEvalAttached {} tkConMain eval
      } elseif {[string match $tkCon(name) $tkCon(app)]} {
	interp alias {} tkConEvalAttached {} uplevel \#0
      } else {
	interp alias {} tkConEvalAttached {} tkConMain interp eval $tkCon(app)
      }
    }
    interp {
      if $tkCon(nontcl) {
	interp alias {} tkConEvalAttached {} tkConEvalSlave
      } else {
	interp alias {} tkConEvalAttached {} tkConEvalSend
      }
    }
    default {
      return -code error "[lindex [info level 0] 0] did not specify\
	a valid type: must be slave or interp"
    }
  }
  return
}

## tkConLoad - sources a file into the console
# ARGS:	fn	- (optional) filename to source in
# Returns:	selected filename ({} if nothing was selected)
## 
proc tkConLoad {{fn {}}} {
  global tkCon
  if {[string match {} $fn] &&
      ([catch {tk_getOpenFile} fn] || [string match {} $fn])} return
  tkConEvalAttached [list source $fn]
}

## tkConSave - saves the console buffer to a file
## This does not eval in a slave because it's not necessary
# ARGS:	w	- console text widget
# 	fn	- (optional) filename to save to
## 
proc tkConSave {{fn {}}} {
  global tkCon
  if {[string match {} $fn] &&
      ([catch {tk_getSaveFile} fn] || [string match {} $fn])} return
  if [catch {open $fn w} fid] {
    return -code error "Save Error: Unable to open '$fn' for writing\n$fid"
  }
  puts $fid [$tkCon(console) get 1.0 end-1c]
  close $fid
}

## tkConMainInit
## This is only called for the main interpreter to include certain procs
## that we don't want to include (or rather, just alias) in slave interps.
##
proc tkConMainInit {} {
  global tkCon

  if ![info exists tkCon(slaves)] {
    array set tkCon [list slave 0 slaves Main name {} interps [tk appname]]
  }
  interp alias {} tkConMain {} tkConInterpEval Main
  interp alias {} tkConSlave {} tkConInterpEval

  ## tkConNew - create new console window
  ## Creates a slave interpreter and sources in this script.
  ## All other interpreters also get a command to eval function in the
  ## new interpreter.
  ## 
  proc tkConNew {} {
    global argv0 argc argv tkCon
    set tmp [interp create Slave[incr tkCon(slave)]]
    lappend tkCon(slaves) $tmp
    load {} Tk $tmp
    lappend tkCon(interps) [$tmp eval [list tk appname "[tk appname] $tmp"]]
    $tmp eval set argc $argc \; set argv [list $argv] \; \
	set argv0 [list $argv0]
    $tmp eval [list set tkCon(name) $tmp]
    $tmp alias tkConDestroy		tkConDestroy $tmp
    $tmp alias tkConNew			tkConNew
    $tmp alias tkConMain		tkConInterpEval Main
    $tmp alias tkConSlave		tkConInterpEval
    $tmp alias tkConInterps		tkConInterps
    $tmp alias tkConStateCheckpoint	tkConStateCheckpoint
    $tmp alias tkConStateCleanup	tkConStateCleanup
    $tmp alias tkConStateCompare	tkConStateCompare
    $tmp alias tkConStateRevert		tkConStateRevert
    $tmp eval [list source $tkCon(SCRIPT)]
    return $tmp
  }

  ## tkConDestroy - destroy console window
  ## This proc should only be called by the main interpreter.  If it is
  ## called from there, it will ask before exiting TkCon.  All others
  ## (slaves) will just have their slave interpreter deleted, closing them.
  ## 
  proc tkConDestroy {{slave {}}} {
    global tkCon
    if [string match {} $slave] {
      ## Main interpreter close request
      if [tk_dialog $tkCon(base).destroyme {Quit TkCon?} \
	      {Closing the Main console will quit TkCon} \
	      warning 0 "Don't Quit" "Quit TkCon"] exit
    } else {
      ## Slave interpreter close request
      set name [tkConInterpEval $slave]
      set tkCon(interps) [lremove $tkCon(interps) [list $name]]
      set tkCon(slaves)  [lremove $tkCon(slaves) [list $slave]]
      interp delete $slave
    }
    tkConStateCleanup $slave
  }

  ## tkConInterpEval - passes evaluation to another named interpreter
  ## If the interpreter is named, but no args are given, it returns the
  ## [tk appname] of that interps master (not the associated eval slave).
  ##
  proc tkConInterpEval {{slave {}} args} {
    if [string match {} $slave] {
      global tkCon
      return $tkCon(slaves)
    } elseif {[string match {[Mm]ain} $slave]} {
      set slave {}
    }
    if [string match {} $args] {
      return [interp eval $slave tk appname]
    } else {
      uplevel \#0 [list interp eval $slave $args]
    }
  }

  proc tkConInterps {{ls {}} {interp {}}} {
    if [string match {} $interp] { lappend ls {} [tk appname] }
    foreach i [interp slaves $interp] {
      if [string comp {} $interp] { set i "$interp $i" }
      if [string comp {} [interp eval $i package provide Tk]] {
	lappend ls $i [interp eval $i tk appname]
      } else {
	lappend ls $i {}
      }
      set ls [tkConInterps $ls $i]
    }
    return $ls
  }

  ##
  ## The following state checkpoint/revert procedures are very sketchy
  ## and prone to problems.  They do not track modifications to currently
  ## existing procedures/variables, and they can really screw things up
  ## if you load in libraries (especially Tk) between checkpoint and
  ## revert.  Only with this knowledge in mind should you use these.
  ##

  ## tkConStateCheckpoint - checkpoints the current state of the system
  ## This allows you to return to this state with tkConStateRevert
  # ARGS:
  ##
  proc tkConStateCheckpoint {app type} {
    global tkCon
    upvar \#0 tkCon($type,$app) a
    if {[array exists a] &&
	[tk_dialog $tkCon(base).warning "Overwrite Previous State?" \
	     "Are you sure you want to lose previously checkpointed\
	     state of $type \"$app\"?" questhead 1 "Do It" "Cancel"]} return
    set a(cmd) [tkConEvalOther $app $type info comm *]
    set a(var) [tkConEvalOther $app $type info vars *]
    return
  }

  ## tkConStateCompare - compare two states and output difference
  # ARGS:
  ##
  proc tkConStateCompare {app type {verbose 0}} {
    global tkCon
    upvar \#0 tkCon($type,$app) a
    if ![array exists a] {
      return -code error "No previously checkpointed state for $type \"$app\""
    }
    set w $tkCon(base).compare
    if [winfo exists $w] {
      $w.text config -state normal
      $w.text delete 1.0 end
    } else {
      toplevel $w
      frame $w.btn
      scrollbar $w.sy -takefocus 0 -bd 1 -command [list $w.text yview]
      text $w.text -font $tkCon(font) -yscrollcommand [list $w.sy set] \
	  -height 12
      pack $w.btn -side bottom -fill x
      pack $w.sy -side right -fill y
      pack $w.text -fill both -expand 1
      button $w.btn.close -text Dismiss -width 11 -command [list destroy $w]
      button $w.btn.check -text Recheckpoint -width 11
      button $w.btn.revert -text Revert -width 11
      button $w.btn.expand -text Verbose -width 11
      button $w.btn.update -text Update -width 11
      pack $w.btn.check $w.btn.revert $w.btn.expand $w.btn.update \
	  $w.btn.close -side left -fill x -padx 4 -pady 2 -expand 1
      $w.text tag config red -foreground red
    }
    wm title $w "Compare State: $type [list $app]"

    $w.btn.check config -command "tkConStateCheckpoint [list $app] $type; \
	tkConStateCompare [list $app] $type $verbose"
    $w.btn.revert config -command "tkConStateRevert [list $app] $type; \
	tkConStateCompare [list $app] $type $verbose"
    $w.btn.update config -command [info level 0]
    if $verbose {
      $w.btn.expand config -text Brief \
	  -command [list tkConStateCompare $app $type 0]
    } else {
      $w.btn.expand config -text Verbose \
	  -command [list tkConStateCompare $app $type 1]
    }
    ## Don't allow verbose mode unless 'dump' exists in $app
    ## We're assuming this is TkCon's dump command
    set hasdump [string comp {} [tkConEvalOther $app $type info comm dump]]
    if $hasdump {
      $w.btn.expand config -state normal
    } else {
      $w.btn.expand config -state disabled
    }

    set cmds [lremove [tkConEvalOther $app $type info comm *] $a(cmd)]
    set vars [lremove [tkConEvalOther $app $type info vars *] $a(var)]

    if {$hasdump && $verbose} {
      set cmds [tkConEvalOther $app $type eval dump c -nocomplain $cmds]
      set vars [tkConEvalOther $app $type eval dump v -nocomplain $vars]
    }
    $w.text insert 1.0 "NEW COMMANDS IN \"$app\":\n" red \
	$cmds {} "\n\nNEW VARIABLES IN \"$app\":\n" red $vars {}

    raise $w
    $w.text config -state disabled
  }

  ## tkConStateRevert - reverts interpreter to previous state
  # ARGS:
  ##
  proc tkConStateRevert {app type} {
    global tkCon
    upvar \#0 tkCon($type,$app) a
    if ![array exists a] {
      return -code error "No previously checkpointed state for $type \"$app\""
    }
    if {![tk_dialog $tkCon(base).warning "Revert State?" \
	      "Are you sure you want to revert the state in $type \"$app\"?" \
	      questhead 1 "Do It" "Cancel"]} {
      foreach i [lremove [tkConEvalOther $app $type info comm *] $a(cmd)] {
	catch {tkConEvalOther $app $type rename $i {}}
      }
      foreach i [lremove [tkConEvalOther $app $type info vars *] $a(var)] {
	catch {tkConEvalOther $app $type unset $i}
      }
    }
  }

  ## tkConStateCleanup - cleans up state information in master array
  #
  ##
  proc tkConStateCleanup {args} {
    global tkCon
    if [string match {} $args] {
      foreach state [array names tkCon slave,*] {
	if ![interp exists [string range $state 6 end]] { unset tkCon($state) }
      }
    } else {
      set app  [lindex $args 0]
      set type [lindex $args 1]
      if [regexp {^(|slave)$} $type] {
	foreach state [concat [array names tkCon slave,$app] \
	    [array names tkCon "slave,$app *"]] {
	  if ![interp exists [string range $state 6 end]] {unset tkCon($state)}
	}
      } else {
	catch {unset tkCon($type,$app)}
      }
    }
  }
}

## tkcon - command that allows control over the console
# ARGS:	totally variable, see internal comments
## 
proc tkcon {cmd args} {
  global tkCon errorInfo
  switch -glob -- $cmd {
    bg* {
      ## 'bgerror' Brings up an error dialog
      set errorInfo [lindex $args 1]
      bgerror [lindex $args 0]
    }
    cl* {
      ## 'close' Closes the console
      tkConDestroy
    }
    con* {
      ## 'console' - passes the args to the text widget of the console.
      eval $tkCon(console) $args
    }
    err* {
      ## Outputs stack caused by last error.
      if {[llength $args]==2} {
	set app  [lindex $args 0]
	set type [lindex $args 1]
	if [catch {tkConEvalOther $app $type set errorInfo} info] {
	  set info "error getting info from $type $app:\n$info"
	}
      } else { set info $tkCon(errorInfo) }
      if [string match {} $info] { set info {errorInfo empty} }
      catch {destroy $tkCon(base).error}
      set w [toplevel $tkCon(base).error]
      wm title $w "TkCon Last Error"
      button $w.close -text Dismiss -command [list destroy $w]
      scrollbar $w.sy -takefocus 0 -bd 1 -command [list $w.text yview]
      text $w.text -font $tkCon(font) -yscrollcommand [list $w.sy set]
      pack $w.close -side bottom -fill x
      pack $w.sy -side right -fill y
      pack $w.text -fill both -expand 1
      $w.text insert 1.0 $info
      $w.text config -state disabled
    }
    fi* {
      ## 'find' string
      tkConFind $tkCon(console) $args
    }
    fo* {
      ## 'font' ?fontname? - gets/sets the font of the console
      if [string comp {} $args] {
	return [$tkCon(console) config -font $args]
      } else {
	return [$tkCon(console) config -font]
      }
    }
    get* {
      ## 'gets' a replacement for [gets stdin varname]
      ## This forces a complete command to be input though
      set old [bind Console <Return>]
      bind Console <Return> { set tkCon(wait) 0 }
      bind Console <KP_Enter> { set tkCon(wait) 0 }
      set w $tkCon(console)
      vwait tkCon(wait)
      set line [tkConCmdGet $tkCon(console)]
      $w insert end \n
      while {![info complete $line]} {
	vwait tkCon(wait)
	set line [tkConCmdGet $tkCon(console)]
	$w insert end \n
      }
      bind Console <Return> $old
      bind Console <KP_Enter> $old
      if [string match {} $args] {
	return $line
      } else {
	upvar [lindex $args 0] data
	set data $line
	return [string length $line]
      }
    }
    hid* {
      ## 'hide' - hides the console with 'withdraw'.
      wm withdraw $tkCon(root)
    }
    his* {
      ## 'history'
      set sub {\2}
      if [string match -n* $args] { append sub "\n" }
      set h [tkConEvalSlave history]
      regsub -all "( *\[0-9\]+  |\t)(\[^\n\]*\n?)" $h $sub h
      return $h
    }
    ico* {
      ## 'iconify' - iconifies the console with 'iconify'.
      wm iconify $tkCon(root)
    }
    mas* - eval {
      ## 'master' - evals contents in master interpreter
      uplevel \#0 $args
    }
    set {
      ## 'set' - set (or get, or unset) simple variables (not whole arrays)
      ## from the master console interpreter
      ## possible formats:
      ##    tkcon set <var>
      ##    tkcon set <var> <value>
      ##    tkcon set <var> <interp> <var1> <var2> w
      ##    tkcon set <var> <interp> <var1> <var2> u
      ##    tkcon set <var> <interp> <var1> <var2> r
      if {[llength $args]==5} {
	## This is for use with 'tkcon upvar' and only works with slaves
	set var [lindex $args 0]
	set i [lindex $args 1]
	set var1 [lindex $args 2]
	set var2 [lindex $args 3]
	if [string compare {} $var2] { append var1 "($var2)" }
	set op [lindex $args 4]
	switch $op {
	  u { uplevel \#0 [list unset $var] }
	  w {
	    return [uplevel \#0 set \{$var\} [interp eval $i set \{$var1\}]]
	  }
	  r {
	    return [interp eval $i set \{$var1\} [uplevel \#0 set \{$var\}]]
	  }
	}
      }
      return [uplevel \#0 set $args]
    }
    sh* - dei* {
      ## 'show|deiconify' - deiconifies the console.
      wm deiconify $tkCon(root)
      raise $tkCon(root)
    }
    ti* {
      ## 'title' ?title? - gets/sets the console's title
      if [string comp {} $args] {
	return [wm title $tkCon(root) $args]
      } else {
	return [wm title $tkCon(root)]
      }
    }
    u* {
      ## 'upvar' masterVar slaveVar
      ## link slave variable slaveVar to the master variable masterVar
      ## only works masters<->slave
      set masterVar [lindex $args 0]
      set slaveVar  [lindex $args 1]
      if [info exists $masterVar] {
	interp eval $tkCon(exec) [list set $myVar [set $masterVar]]
      } else {
	catch {interp eval $tkCon(exec) [list unset $myVar]}
      }
      interp eval $tkCon(exec) [list trace variable $myVar rwu \
	  [list tkcon set $masterVar $tkCon(exec)]]
      return
    }
    v* {
      return $tkCon(version)
    }
    default {
      ## tries to determine if the command exists, otherwise throws error
      set new tkCon[string toup [string index $cmd 0]][string range $cmd 1 end]
      if [string comp {} [info command $new]] {
	uplevel \#0 $new $args
      } else {
	return -code error "bad option \"$cmd\": must be\
	    [join [lsort [list attach close console destroy font hide \
	    iconify load main master new save show slave deiconify \
	    version title bgerror]] {, }]"
      }
    }
  }
}

##
## Some procedures to make up for lack of built-in shell commands
##

## puts
## This allows me to capture all stdout/stderr to the console window
# ARGS:	same as usual	
# Outputs:	the string with a color-coded text tag
## 
if ![catch {rename puts tcl_puts}] {
  proc puts args {
    set len [llength $args]
    if {$len==1} {
      eval tkcon console insert output $args stdout {\n} stdout
      tkcon console see output
    } elseif {$len==2 && \
	[regexp {(stdout|stderr|-nonewline)} [lindex $args 0] junk tmp]} {
      if [string comp $tmp -nonewline] {
	eval tkcon console insert output [lreplace $args 0 0] $tmp {\n} $tmp
      } else {
	eval tkcon console insert output [lreplace $args 0 0] stdout
      }
      tkcon console see output
    } elseif {$len==3 && \
	[regexp {(stdout|stderr)} [lreplace $args 2 2] junk tmp]} {
      if [string comp [lreplace $args 1 2] -nonewline] {
	eval tkcon console insert output [lrange $args 1 1] $tmp
      } else {
	eval tkcon console insert output [lreplace $args 0 1] $tmp
      }
      tkcon console see output
    } else {
      global errorCode errorInfo
      if [catch "tcl_puts $args" msg] {
	regsub tcl_puts $msg puts msg
	regsub -all tcl_puts $errorInfo puts errorInfo
      }
      return -errorcode $errorCode $msg
      #eval tcl_puts $args
    }
    if $len update
  }
}

## echo
## Relaxes the one string restriction of 'puts'
# ARGS:	any number of strings to output to stdout
##
proc echo args { puts [concat $args] }

## clear - clears the buffer of the console (not the history though)
## This is executed in the parent interpreter
## 
proc clear {{pcnt 100}} {
  if {![regexp {^[0-9]*$} $pcnt] || $pcnt < 1 || $pcnt > 100} {
    return -code error \
	"invalid percentage to clear: must be 1-100 (100 default)"
  } elseif {$pcnt == 100} {
    tkcon console delete 1.0 end
  } else {
    set tmp [expr $pcnt/100.0*[tkcon console index end]]
    tkcon console delete 1.0 "$tmp linestart"
  }
}

## alias - akin to the csh alias command
## If called with no args, then it dumps out all current aliases
## If called with one arg, returns the alias of that arg (or {} if none)
# ARGS:	newcmd	- (optional) command to bind alias to
# 	args	- command and args being aliased
## 
proc alias {{newcmd {}} args} {
  if [string match {} $newcmd] {
    set res {}
    foreach a [interp aliases] {
      lappend res [list $a -> [interp alias {} $a]]
    }
    return [join $res \n]
  } elseif {[string match {} $args]} {
    interp alias {} $newcmd
  } else {
    eval interp alias [list {} $newcmd {}] $args
  }
}

## unalias - unaliases an alias'ed command
# ARGS:	cmd	- command to unbind as an alias
## 
proc unalias {cmd} {
  interp alias {} $cmd {}
}

## dump - outputs variables/procedure/widget info in source'able form.
## Accepts glob style pattern matching for the names
# ARGS:	type	- type of thing to dump: must be variable, procedure, widget
# OPTS: -nocomplain
#		don't complain if no vars match something
#	-filter pattern
#		specifies a glob filter pattern to be used by the variable
#		method as an array filter pattern (it filters down for
#		nested elements) and in the widget method as a config
#		option filter pattern
#	--	forcibly ends options recognition
# Returns:	the values of the requested items in a 'source'able form
## 
proc dump {type args} {
  set whine 1
  set code  ok
  while {[string match -* $args]} {
    switch -glob -- [lindex $args 0] {
      -n* { set whine 0; set args [lreplace $args 0 0] }
      -f* { set fltr [lindex $args 1]; set args [lreplace $args 0 1] }
      --  { set args [lreplace $args 0 0]; break }
      default { return -code error "unknown option \"[lindex $args 0]\"" }
    }
  }
  if {$whine && [string match {} $args]} {
    return -code error "wrong \# args: [lindex [info level 0] 0]\
	?-nocomplain? ?-filter pattern? ?--? pattern ?pattern ...?"
  }
  set res {}
  switch -glob -- $type {
    c* {
      # command
      # outpus commands by figuring out, as well as possible, what it is
      # this does not attempt to auto-load anything
      foreach arg $args {
	if [string comp {} [set cmds [info comm $arg]]] {
	  foreach cmd [lsort $cmds] {
	    if {[lsearch -exact [interp aliases] $cmd] > -1} {
	      append res "\#\# ALIAS:   $cmd => [interp alias {} $cmd]\n"
	    } elseif {[string comp {} [info procs $cmd]]} {
	      if {[catch {dump p -- $cmd} msg] && $whine} { set code error }
	      append res $msg\n
	    } else {
	      append res "\#\# COMMAND: $cmd\n"
	    }
	  }
	} elseif $whine {
	  append res "\#\# No known command $arg\n"
	  set code error
	}
      }
    }
    v* {
      # variable
      # outputs variables value(s), whether array or simple.
      if ![info exists fltr] { set fltr * }
      foreach arg $args {
	if {[string match {} [set vars [uplevel info vars [list $arg]]]]} {
	  if {[uplevel info exists $arg]} {
	    set vars $arg
	  } elseif $whine {
	    append res "\#\# No known variable $arg\n"
	    set code error
	    continue
	  } else continue
	}
	foreach var [lsort $vars] {
	  upvar $var v
	  if {[array exists v]} {
	    set nest {}
	    append res "array set $var \{\n"
	    foreach i [lsort [array names v $fltr]] {
	      upvar 0 v\($i\) __ary
	      if {[array exists __ary]} {
		append nest "\#\# NESTED ARRAY ELEMENT: $i\n"
		append nest "upvar 0 [list $var\($i\)] __ary;\
		    [dump v -filter $fltr __ary]\n"
	      } else {
		append res "    [list $i]\t[list $v($i)]\n"
	      }
	    }
	    append res "\}\n$nest"
	  } else {
	    append res [list set $var $v]\n
	  }
	}
      }
    }
    p* {
      # procedure
      foreach arg $args {
	if {[string comp {} [set ps [info proc $arg]]] ||
	    ([auto_load $arg] &&
	     [string comp {} [set ps [info proc $arg]]])} {
	  foreach p [lsort $ps] {
	    set as {}
	    foreach a [info args $p] {
	      if {[info default $p $a tmp]} {
		lappend as [list $a $tmp]
	      } else {
		lappend as $a
	      }
	    }
	    append res [list proc $p $as [info body $p]]\n
	  }
	} elseif $whine {
	  append res "\#\# No known proc $arg\n"
	  set code error
	}
      }
    }
    w* {
      # widget
      ## The user should have Tk loaded
      if [string match {} [info command winfo]] {
	return -code error "winfo not present, cannot dump widgets"
      }
      if ![info exists fltr] { set fltr .* }
      foreach arg $args {
	if [string comp {} [set ws [info command $arg]]] {
	  foreach w [lsort $ws] {
	    if [winfo exists $w] {
	      if [catch {$w configure} cfg] {
		append res "\#\# Widget $w does not support configure method"
		set code error
	      } else {
		append res "\#\# [winfo class $w] $w\n$w configure"
		foreach c $cfg {
		  if {[llength $c] != 5} continue
		  if {[regexp -nocase -- $fltr $c]} {
		    append res " \\\n\t[list [lindex $c 0] [lindex $c 4]]"
		  }
		}
		append res \n
	      }
	    }
	  }
	} elseif $whine {
	  append res "\#\# No known widget $arg\n"
	  set code error
	}
      }
    }
    default {
      return -code error "bad [lindex [info level 0] 0] option\
	\"$type\":\ must be procedure, variable, widget"
    }
  }
  return -code $code [string trimr $res \n]
}

## idebug - interactive debugger
# ARGS:	opt
#
##
proc idebug {opt args} {
  global IDEBUG

  if ![info exists IDEBUG(on)] { array set IDEBUG { on 0 id * debugging 0 } }
  set level [expr [info level]-1]
  switch -glob -- $opt {
    on	{
      if [string comp {} $args] { set IDEBUG(id) $args }
      return [set IDEBUG(on) 1]
    }
    off	{ return [set IDEBUG(on) 0] }
    id  {
      if [string match {} $args] {
	return $IDEBUG(id)
      } else { return [set IDEBUG(id) $args] }
    }
    break {
      if {!$IDEBUG(on) || $IDEBUG(debugging) || ([string comp {} $args] \
	  && ![string match $IDEBUG(id) $args]) || [info level]<1} return
      set IDEBUG(debugging) 1
      puts stderr "idebug at level \#$level: [lindex [info level -1] 0]"
      set tkcon [string comp {} [info command tkcon]]
      if $tkcon {
	tkcon show
	tkcon master eval set tkCon(prompt2) \$tkCon(prompt1)
	tkcon master eval set tkCon(prompt1) \$tkCon(debugPrompt)
	set slave [tkcon set tkCon(exec)]
	set event [tkcon set tkCon(event)]
	tkcon set tkCon(exec) [tkcon master interp create debugger]
	tkcon set tkCon(event) 1
      }
      set max $level
      while 1 {
	set err {}
	if $tkcon {
	  tkcon evalSlave set level $level
	  tkcon prompt
	  set line [tkcon gets]
	  tkcon console mark set output end
	} else {
	  puts -nonewline stderr "(level \#$level) debug > "
	  gets stdin line
	  while {![info complete $line]} {
	    puts -nonewline "> "
	    append line "\n[gets stdin]"
	  }
	}
	if [string match {} $line] continue
	set key [lindex $line 0]
	if ![regexp {^([\#-]?[0-9]+)} [lreplace $line 0 0] lvl] {
	  set lvl \#$level
	}
	set res {}; set c 0
	switch -- $key {
	  + {
	    ## Allow for jumping multiple levels
	    if {$level < $max} { idebug trace [incr level] $level 0 VERBOSE }
	  }
	  - {
	    ## Allow for jumping multiple levels
	    if {$level > 1} { idebug trace [incr level -1] $level 0 VERBOSE }
	  }
	  . { set c [catch { idebug trace $level $level 0 VERBOSE } res] }
	  v { set c [catch { idebug show vars $lvl } res] }
	  V { set c [catch { idebug show vars $lvl VERBOSE } res] }
	  l { set c [catch { idebug show locals $lvl } res] }
	  L { set c [catch { idebug show locals $lvl VERBOSE } res] }
	  g { set c [catch { idebug show globals $lvl } res] }
	  G { set c [catch { idebug show globals $lvl VERBOSE } res] }
	  t { set c [catch { idebug trace 1 $max $level } res] }
	  T { set c [catch { idebug trace 1 $max $level VERBOSE } res] }
	  b { set c [catch { idebug body $lvl } res] }
	  o { set res [set IDEBUG(on) [expr !$IDEBUG(on)]] }
	  h - ?	{
	    puts stderr "    +		Move down in call stack
    -		Move up in call stack
    .		Show current proc name and params

    v		Show names of variables currently in scope
    V		Show names of variables currently in scope with values
    l		Show names of local (transient) variables
    L		Show names of local (transient) variables with values
    g		Show names of declared global variables
    G		Show names of declared global variables with values
    t		Show a stack trace
    T		Show a verbose stack trace

    b		Show body of current proc
    o		Toggle on/off any further debugging
    c,q		Continue regular execution (Quit debugger)
    h,?		Print this help
    default	Evaluate line at current level (\#$level)"
	  }
	  c - q break
	  default { set c [catch {uplevel \#$level $line} res] }
	}
	if $tkcon {
	  tkcon set tkCon(event) \
	      [tkcon evalSlave eval history add [list $line] \; history nextid]
	}
	if $c { puts stderr $res } elseif {[string comp {} $res]} { puts $res }
      }
      set IDEBUG(debugging) 0
      if $tkcon {
	tkcon master interp delete debugger
	tkcon master eval set tkCon(prompt1) \$tkCon(prompt2)
	tkcon set tkCon(exec) $slave
	tkcon set tkCon(event) $event
	tkcon prompt
      }
    }
    bo* {
      if [regexp {^([\#-]?[0-9]+)} $args level] {
	return [uplevel $level { dump com -no [lindex [info level 0] 0] }]
      }
    }
    t* {
      if {[llength $args]<2} return
      set min [set max [set lvl $level]]
      if ![regexp {^\#?([0-9]+)? ?\#?([0-9]+) ?\#?([0-9]+)? ?(VERBOSE)?} \
	  $args junk min max lvl verbose] return
      for {set i $max} {
	$i>=$min && ![catch {uplevel \#$i info level 0} info]
      } {incr i -1} {
	if {$i==$lvl} {
	  puts -nonewline stderr "* \#$i:\t"
	} else {
	  puts -nonewline stderr "  \#$i:\t"
	}
	set name [lindex $info 0]
	if {[string comp VERBOSE $verbose] || \
	    [string match {} [info procs $name]]} {
	  puts $info
	} else {
	  puts "proc $name {[info args $name]} { ... }"
	  set idx 0
	  foreach arg [info args $name] {
	    if [string match args $arg] {
	      puts "\t$arg = [lrange $info [incr idx] end]"; break
	    } else {
	      puts "\t$arg = [lindex $info [incr idx]]"
	    }
	  }
	}
      }
    }
    s* {
      #var, local, global
      set level \#$level
      if ![regexp {^([vgl][^ ]*) ?([\#-]?[0-9]+)? ?(VERBOSE)?} \
	  $args junk type level verbose] return
      switch -glob -- $type {
	v* { set vars [uplevel $level {lsort [info vars]}] }
	l* { set vars [uplevel $level {lsort [info locals]}] }
	g* { set vars [lremove [uplevel $level {info vars}] \
	    [uplevel $level {info locals}]] }
      }
      if [string match VERBOSE $verbose] {
	return [uplevel $level dump var -nocomplain $vars]
      } else {
	return $vars
      }
    }
    e* - pu* {
      if {[llength $opt]==1 && [catch {lindex [info level -1] 0} id]} {
	set id [lindex [info level 0] 0]
      } else {
	set id [lindex $opt 1]
      }
      if {$IDEBUG(on) && [string match $IDEBUG(id) $id]} {
	if [string match e* $opt] {
	  puts [concat $args]
	} else { eval puts $args }
      }
    }
    default {
      return -code error "bad [lindex [info level 0] 0] option \"$opt\":\
	  must be [join [lsort [list on off id break print body trace \
	  show puts echo]] {, }]"
    }
  }
}

## observe - like trace, but not
# ARGS:	opt	- option
#	name	- name of variable or command
##
proc observe {opt name args} {
  global tcl_observe
  switch -glob -- $opt {
    co* {
      if [regexp {^(catch|lreplace|set|puts|for|incr|info|uplevel)$} $name] {
	return -code error \
	    "cannot observe \"$name\": infinite eval loop will occur"
      }
      set old ${name}@
      while {[string comp {} [info command $old]]} { append old @ }
      rename $name $old
      set max 4
      regexp {^[0-9]+} $args max
      ## idebug trace could be used here
      proc $name args "
      for {set i \[info level\]; set max \[expr \[info level\]-$max\]} {
	\$i>=\$max && !\[catch {uplevel \#\$i info level 0} info\]
      } {incr i -1} {
	puts -nonewline stderr \"  \#\$i:\t\"
	puts \$info
      }
      uplevel \[lreplace \[info level 0\] 0 0 $old\]
      "
      set tcl_observe($name) $old
    }
    cd* {
      if {[info exists tcl_observe($name)] && [catch {
	rename $name {}
	rename $tcl_observe($name) $name
	unset tcl_observe($name)
      } err]} { return -code error $err }
    }
    ci* {
      ## What a useless method...
      if [info exists tcl_observe($name)] {
	set i $tcl_observe($name)
	set res "\"$name\" observes true command \"$i\""
	while {[info exists tcl_observe($i)]} {
	  append res "\n\"$name\" observes true command \"$i\""
	  set i $tcl_observe($name)
	}
	return $res
      }
    }
    va* - vd* {
      set type [lindex $args 0]
      set args [lrange $args 1 end]
      if ![regexp {^[rwu]} $type type] {
	return -code error "bad [lindex [info level 0] 0] $opt type\
	    \"$type\": must be read, write or unset"
      }
      if [string match {} $args] { set args observe_var }
      uplevel [list trace $opt $name $type $args]
    }
    vi* {
      uplevel [list trace vinfo $name]
    }
    default {
      return -code error "bad [lindex [info level 0] 0] option\
	 \"[lindex $args 0]\": must be [join [lsort [list procedure \
	 pdelete pinfo variable vdelete vinfo]] {, }]"
    }
  }
}

## observe_var - auxilary function for observing vars, called by trace
## via observe
# ARGS:	name	- variable name
#	el	- array element name, if any
#	op	- operation type (rwu)
##
proc observe_var {name el op} {
  if [string match u $op] {
    if [string comp {} $el] {
      puts "unset \"$name\($el\)\""
    } else {
      puts "unset \"$name\""
    }
  } else {
    upvar $name $name
    if [info exists $name\($el\)] {
      puts [dump v $name\($el\)]
    } else {
      puts [dump v $name]
    }
  }
}

## which - tells you where a command is found
# ARGS:	cmd	- command name
# Returns:	where command is found (internal / external / unknown)
## 
proc which cmd {
  if {[string comp {} [info commands $cmd]] ||
      ([auto_load $cmd] && [string comp {} [info commands $cmd]])} {
    if {[lsearch -exact [interp aliases] $cmd] > -1} {
      return "$cmd:\taliased to [alias $cmd]"
    } elseif {[string comp {} [info procs $cmd]]} {
      return "$cmd:\tinternal proc"
    } else {
      return "$cmd:\tinternal command"
    }
  } elseif {[string comp {} [auto_execok $cmd]]} {
    return [auto_execok $cmd]
  } else {
    return -code error "$cmd:\tunknown command"
  }
}

## auto_execpath - tells you where an external command is found
## Only a slight modification from core auto_execok proc
# ARGS:	cmd	- command name
# Returns:	where command is found or {} if not found
## 
if {[info tclversion]<7.6} {
if {[string match $tcl_platform(platform) windows]} {

# auto_execok --
#
# Returns string that indicates name of program to execute if 
# name corresponds to a shell builtin or an executable in the
# Windows search path, or "" otherwise.  Builds an associative 
# array auto_execs that caches information about previous checks, 
# for speed.
#
# Arguments: 
# name -			Name of a command.

# Windows version.
#
# Note that info executable doesn't work under Windows, so we have to
# look for files with .exe, .com, or .bat extensions.  Also, the path
# may be in the Path or PATH environment variables, and path
# components are separated with semicolons, not colons as under Unix.
#
proc auto_execok name {
    global auto_execs env tcl_platform

    if [info exists auto_execs($name)] {
	return $auto_execs($name)
    }
    set auto_execs($name) ""

    if {[lsearch -exact {cls copy date del erase dir echo mkdir md rename 
	    ren rmdir rd time type ver vol} $name] != -1} {
	if {[info exists env(COMSPEC)]} {
	    set comspec $env(COMSPEC) 
	} elseif {[info exists env(ComSpec)]} {
	    set comspec $env(ComSpec)
	} elseif {$tcl_platform(os) == "Windows NT"} {
	    set comspec "cmd.exe"
	} else {
	    set comspec "command.com"
	}
	return [set auto_execs($name) [list $comspec /c $name]]
    }

    if {[llength [file split $name]] != 1} {
	foreach ext {{} .com .exe .bat} {
	    set file ${name}${ext}
	    if {[file exists $file] && ![file isdirectory $file]} {
		return [set auto_execs($name) $file]
	    }
	}
	return ""
    }

    set path "[file dirname [info nameof]];.;"
    if {[info exists env(WINDIR)]} {
	set windir $env(WINDIR) 
    } elseif {[info exists env(windir)]} {
	set windir $env(windir)
    }
    if {[info exists windir]} {
	if {$tcl_platform(os) == "Windows NT"} {
	    append path "$windir/system32;"
	}
	append path "$windir/system;$windir;"
    }

    if {! [info exists env(PATH)]} {
	if [info exists env(Path)] {
	    append path $env(Path)
	} else {
	    return ""
	}
    } else {
	append path $env(PATH)
    }

    foreach dir [split $path {;}] {
	if {$dir == ""} {
	    set dir .
	}
	foreach ext {{} .com .exe .bat} {
	    set file [file join $dir ${name}${ext}]
	    if {[file exists $file] && ![file isdirectory $file]} {
		return [set auto_execs($name) $file]
	    }
	}
    }
    return ""
}

} else {

# auto_execok --
#
# Returns string that indicates name of program to execute if 
# name corresponds to an executable in the path. Builds an associative 
# array auto_execs that caches information about previous checks, 
# for speed.
#
# Arguments: 
# name -			Name of a command.

# Unix version.
#
proc auto_execok name {
    global auto_execs env

    if [info exists auto_execs($name)] {
	return $auto_execs($name)
    }
    set auto_execs($name) ""
    if {[llength [file split $name]] != 1} {
	if {[file executable $name] && ![file isdirectory $name]} {
	    set auto_execs($name) $name
	}
	return $auto_execs($name)
    }
    foreach dir [split $env(PATH) :] {
	if {$dir == ""} {
	    set dir .
	}
	set file [file join $dir $name]
	if {[file executable $file] && ![file isdirectory $file]} {
	    set auto_execs($name) $file
	    return $file
	}
    }
    return ""
}

}
}

## dir - directory list
# ARGS:	args	- names/glob patterns of directories to list
# OPTS:	-all	- list hidden files as well (Unix dot files)
#	-long	- list in full format "permissions size date filename"
#	-full	- displays / after directories and link paths for links
# Returns:	a directory listing
## 
proc dir {args} {
  array set s {
    all 0 full 0 long 0
    0 --- 1 --x 2 -w- 3 -wx 4 r-- 5 r-x 6 rw- 7 rwx
  }
  while {[string match \-* [lindex $args 0]]} {
    set str [lindex $args 0]
    set args [lreplace $args 0 0]
    switch -glob -- $str {
      -a* {set s(all) 1} -f* {set s(full) 1}
      -l* {set s(long) 1} -- break
      default {
	return -code error \
	    "unknown option \"$str\", should be one of: -all, -full, -long"
      }
    }
  }
  set sep [string trim [file join . .] .]
  if [string match {} $args] { set args . }
  foreach arg $args {
    if {[file isdir $arg]} {
      set arg [string trimr $arg $sep]$sep
      if $s(all) {
	lappend out [list $arg [lsort [glob -nocomplain -- $arg.* $arg*]]]
      } else {
	lappend out [list $arg [lsort [glob -nocomplain -- $arg*]]]
      }
    } else {
      lappend out [list [file dirname $arg]$sep \
		       [lsort [glob -nocomplain -- $arg]]]
    }
  }
  if $s(long) {
    set old [clock scan {1 year ago}]
    set fmt "%s%9d %s %s\n"
    foreach o $out {
      set d [lindex $o 0]
      append res $d:\n
      foreach f [lindex $o 1] {
	file lstat $f st
	set f [file tail $f]
	if $s(full) {
	  switch -glob $st(type) {
	    d* { append f $sep }
	    l* { append f "@ -> [file readlink $d$sep$f]" }
	    default { if [file exec $d$sep$f] { append f * } }
	  }
	}
	if [string match file $st(type)] {
	  set mode -
	} else {
	  set mode [string index $st(type) 0]
	}
	foreach j [split [format %o [expr $st(mode)&0777]] {}] {
	  append mode $s($j)
	}
	if {$st(mtime)>$old} {
	  set cfmt {%b %d %H:%M}
	} else {
	  set cfmt {%b %d  %Y}
	}
	append res [format $fmt $mode $st(size) \
			[clock format $st(mtime) -format $cfmt] $f]
      }
      append res \n
    }
  } else {
    foreach o $out {
      set d [lindex $o 0]
      append res $d:\n
      set i 0
      foreach f [lindex $o 1] {
	if {[string len [file tail $f]] > $i} {
	  set i [string len [file tail $f]]
	}
      }
      set i [expr $i+2+$s(full)]
      ## This gets the number of cols in the TkCon console widget
      set j [expr [tkcon master set tkCon(cols)]/$i]
      set k 0
      foreach f [lindex $o 1] {
	set f [file tail $f]
	if $s(full) {
	  switch -glob [file type $d$sep$f] {
	    d* { append f $sep }
	    l* { append f @ }
	    default { if [file exec $d$sep$f] { append f * } }
	  }
	}
	append res [format "%-${i}s" $f]
	if {[incr k]%$j == 0} {set res [string trimr $res]\n}
      }
      append res \n\n
    }
  }
  return [string trimr $res]
}
interp alias {} ls {} dir

## tclindex - creates the tclIndex file
# OPTS:	-ext	- extensions to auto index (defaults to *.tcl)
#	-pkg	- whether to create a pkgIndex.tcl file
#	-idx	- whether to create a tclIndex file
# ARGS:	args	- directories to auto index (defaults to pwd)
# Outputs:	tclIndex file to each directory
##
proc tclindex args {
  set truth {^(1|yes|true|on)$}; set pkg 0; set idx 1;
  while {[regexp -- {^-[^ ]+} $args opt] && [string comp {} $args]} {
    switch -glob -- $opt {
      --  { set args [lreplace $args 0 0]; break }
      -e* {
	set ext [lindex $args 1]
	set args [lreplace $args 0 1]
      }
      -p* {
	set pkg [regexp -nocase $truth [lindex $args 1]]
	set args [lreplace $args 0 1]
      }
      -i* {
	set idx [regexp -nocase $truth [lindex $args 1]]
	set args [lreplace $args 0 1]
      }
      default {
	return -code error "bad option \"$opt\": must be one of\
	    [join [lsort [list -- -extension -package -index]] {, }]"
      }
    }
  }
  if ![info exists ext] {
    set ext {*.tcl}
    if $pkg { lappend ext *[info sharedlibextension] }
  }
  if [string match {} $args] {
    if $idx { eval auto_mkindex [list [pwd]] $ext }
    if $pkg { eval pkg_mkIndex [list [pwd]] $ext }
  } else {
    foreach dir $args {
      if [file isdir $dir] {
	if $idx { eval auto_mkindex [list [pwd]] $ext }
	if $pkg { eval pkg_mkIndex [list [pwd]] $ext }
      }
    }
  }
}

## lremove - remove items from a list
# OPTS:	-all	remove all instances of each item
# ARGS:	l	a list to remove items from
#	args	items to remove
##
proc lremove {args} {
  set all 0
  if [string match \-a* [lindex $args 0]] {
    set all 1
    set args [lreplace $args 0 0]
  }
  set l [lindex $args 0]
  eval append is [lreplace $args 0 0]
  foreach i $is {
    if {[set ix [lsearch -exact $l $i]] == -1} continue
    set l [lreplace $l $ix $ix]
    if $all {
      while {[set ix [lsearch -exact $l $i]] != -1} {
	set l [lreplace $l $ix $ix]
      }
    }
  }
  return $l
}

## Unknown changed to get output into tkCon window
# unknown:
# Invoked automatically whenever an unknown command is encountered.
# Works through a list of "unknown handlers" that have been registered
# to deal with unknown commands.  Extensions can integrate their own
# handlers into the "unknown" facility via "unknown_handle".
#
# If a handler exists that recognizes the command, then it will
# take care of the command action and return a valid result or a
# Tcl error.  Otherwise, it should return "-code continue" (=2)
# and responsibility for the command is passed to the next handler.
#
# Arguments:
# args -	A list whose elements are the words of the original
#		command, including the command name.

proc unknown args {
    global unknown_handler_order unknown_handlers errorInfo errorCode

    #
    # Be careful to save error info now, and restore it later
    # for each handler.  Some handlers generate their own errors
    # and disrupt handling.
    #
    set savedErrorCode $errorCode
    set savedErrorInfo $errorInfo

    if {![info exists unknown_handler_order] || ![info exists unknown_handlers]} {
	set unknown_handlers(tcl) tcl_unknown
	set unknown_handler_order tcl
    }

    foreach handler $unknown_handler_order {
        set status [catch {uplevel $unknown_handlers($handler) $args} result]

        if {$status == 1} {
            #
            # Strip the last five lines off the error stack (they're
            # from the "uplevel" command).
            #
            set new [split $errorInfo \n]
            set new [join [lrange $new 0 [expr [llength $new] - 6]] \n]
            return -code $status -errorcode $errorCode \
                -errorinfo $new $result

        } elseif {$status != 4} {
            return -code $status $result
        }

        set errorCode $savedErrorCode
        set errorInfo $savedErrorInfo
    }

    set name [lindex $args 0]
    return -code error "invalid command name \"$name\""
}

# tcl_unknown:
# Invoked when a Tcl command is invoked that doesn't exist in the
# interpreter:
#
#	1. See if the autoload facility can locate the command in a
#	   Tcl script file.  If so, load it and execute it.
#	2. If the command was invoked interactively at top-level:
#	    (a) see if the command exists as an executable UNIX program.
#		If so, "exec" the command.
#	    (b) see if the command requests csh-like history substitution
#		in one of the common forms !!, !<number>, or ^old^new.  If
#		so, emulate csh's history substitution.
#	    (c) see if the command is a unique abbreviation for another
#		command.  If so, invoke the command.
#
# Arguments:
# args -	A list whose elements are the words of the original
#		command, including the command name.

proc tcl_unknown args {
  global auto_noexec auto_noload env unknown_pending tcl_interactive tkCon
  global errorCode errorInfo

  # Save the values of errorCode and errorInfo variables, since they
  # may get modified if caught errors occur below.  The variables will
  # be restored just before re-executing the missing command.

  set savedErrorCode $errorCode
  set savedErrorInfo $errorInfo
  set name [lindex $args 0]
  if ![info exists auto_noload] {
    #
    # Make sure we're not trying to load the same proc twice.
    #
    if [info exists unknown_pending($name)] {
      unset unknown_pending($name)
      if {[array size unknown_pending] == 0} {
	unset unknown_pending
      }
      return -code error "self-referential recursion in \"unknown\" for command \"$name\"";
    }
    set unknown_pending($name) pending;
    set ret [catch {auto_load $name} msg]
    unset unknown_pending($name);
    if $ret {
      return -code $ret -errorcode $errorCode \
	  "error while autoloading \"$name\": $msg"
    }
    if ![array size unknown_pending] {
      unset unknown_pending
    }
    if $msg {
      set errorCode $savedErrorCode
      set errorInfo $savedErrorInfo
      set code [catch {uplevel $args} msg]
      if {$code ==  1} {
	#
	# Strip the last five lines off the error stack (they're
	# from the "uplevel" command).
	#

	set new [split $errorInfo \n]
	set new [join [lrange $new 0 [expr [llength $new] - 6]] \n]
	return -code error -errorcode $errorCode \
	    -errorinfo $new $msg
      } else {
	return -code $code $msg
      }
    }
  }
  if {[info level] == 1 && [string match {} [info script]] \
	  && [info exists tcl_interactive] && $tcl_interactive} {
    if ![info exists auto_noexec] {
      set new [auto_execok $name]
      if {$new != ""} {
	set errorCode $savedErrorCode
	set errorInfo $savedErrorInfo
	return [uplevel exec [list $new] [lrange $args 1 end]]
	#return [uplevel exec >&@stdout <@stdin $new [lrange $args 1 end]]
      }
    }
    set errorCode $savedErrorCode
    set errorInfo $savedErrorInfo
    ##
    ## History substitution moved into tkConEvalCmd
    ##
    set cmds [info commands $name*]
    if {[llength $cmds] == 1} {
      return [uplevel [lreplace $args 0 0 $cmds]]
    }
    if {[llength $cmds]} {
      if {$name == ""} {
	return -code error "empty command name \"\""
      } else {
	return -code error \
	    "ambiguous command name \"$name\": [lsort $cmds]"
      }
    }
  }
  return -code continue
}

proc tkConBindings {} {
  global tkCon tcl_platform tk_version

  #-----------------------------------------------------------------------
  # Elements of tkPriv that are used in this file:
  #
  # char -		Character position on the line;  kept in order
  #			to allow moving up or down past short lines while
  #			still remembering the desired position.
  # mouseMoved -	Non-zero means the mouse has moved a significant
  #			amount since the button went down (so, for example,
  #			start dragging out a selection).
  # prevPos -		Used when moving up or down lines via the keyboard.
  #			Keeps track of the previous insert position, so
  #			we can distinguish a series of ups and downs, all
  #			in a row, from a new up or down.
  # selectMode -	The style of selection currently underway:
  #			char, word, or line.
  # x, y -		Last known mouse coordinates for scanning
  #			and auto-scanning.
  #-----------------------------------------------------------------------

  switch -glob $tcl_platform(platform) {
    win* { set tkCon(meta) Alt }
    mac* { set tkCon(meta) Command }
    default { set tkCon(meta) Meta }
  }

  ## <<TkCon_Exit>>
  bind $tkCon(root) <Control-q> exit
  ## <<TkCon_New>>
  bind $tkCon(root) <Control-N> { tkConNew }
  ## <<TkCon_Close>>
  bind $tkCon(root) <Control-w> { tkConDestroy }
  ## <<TkCon_About>>
  bind $tkCon(root) <Control-A> { tkConAbout }
  ## <<TkCon_Help>>
  bind $tkCon(root) <Control-H> { tkConHelp }
  ## <<TkCon_Find>>
  bind $tkCon(root) <Control-F> { tkConFindBox $tkCon(console) }
  ## <<TkCon_Slave>>
  bind $tkCon(root) <Control-Key-1> {
    tkConAttach {}
    tkConPrompt "\n" [tkConCmdGet $tkCon(console)]
  }
  ## <<TkCon_Master>>
  bind $tkCon(root) <Control-Key-2> {
    if [string comp {} $tkCon(name)] {
      tkConAttach $tkCon(name)
    } else {
      tkConAttach Main
    }
    tkConPrompt "\n" [tkConCmdGet $tkCon(console)]
  }
  ## <<TkCon_Main>>
  bind $tkCon(root) <Control-Key-3> {
    tkConAttach Main
    tkConPrompt "\n" [tkConCmdGet $tkCon(console)]
  }

  ## Menu items need null PostCon bindings to avoid the TagProc
  ##
  foreach ev [bind $tkCon(root)] {
    bind PostCon $ev {
      # empty
    }
  }

  # tkConClipboardKeysyms --
  # This procedure is invoked to identify the keys that correspond to
  # the "copy", "cut", and "paste" functions for the clipboard.
  #
  # Arguments:
  # copy -	Name of the key (keysym name plus modifiers, if any,
  #		such as "Meta-y") used for the copy operation.
  # cut -		Name of the key used for the cut operation.
  # paste -	Name of the key used for the paste operation.

  proc tkConClipboardKeysyms {copy cut paste} {
    bind Console <$copy>	{tkConCopy %W}
    bind Console <$cut>		{tkConCut %W}
    bind Console <$paste>	{tkConPaste %W}
  }

  proc tkConCut w {
    if [string match $w [selection own -displayof $w]] {
      clipboard clear -displayof $w
      catch {
	clipboard append -displayof $w [selection get -displayof $w]
	if [$w compare sel.first >= limit] {$w delete sel.first sel.last}
      }
    }
  }
  proc tkConCopy w {
    if [string match $w [selection own -displayof $w]] {
      clipboard clear -displayof $w
      catch {clipboard append -displayof $w [selection get -displayof $w]}
    }
  }

  proc tkConPaste w {
    if ![catch {selection get -displayof $w -selection CLIPBOARD} tmp] {
      if [$w compare insert < limit] {$w mark set insert end}
      $w insert insert $tmp
      $w see insert
      if [string match *\n* $tmp] {tkConEval $w}
    }
  }

  ## Get all Text bindings into Console except Unix cut/copy/paste
  ## and newline insertion
  foreach ev [lremove [bind Text] {<Control-Key-y> <Control-Key-w> \
      <Meta-Key-w> <Control-Key-o> <Control-Key-v> <Control-Key-c> \
      <Control-Key-x>}] {
    bind Console $ev [bind Text $ev]
  }

  ## Redefine for Console what we need
  ##
  if [string compare {} [info command event]] {
    event delete <<Paste>> <Control-V>
    tkConClipboardKeysyms <Copy> <Cut> <Paste>
  } else {
    tkConClipboardKeysyms F16 F20 F18
    tkConClipboardKeysyms Control-c Control-x Control-v
  }

  bind Console <Insert> {catch {tkConInsert %W [selection get -displayof %W]}}

  bind Console <Triple-1> {+
    catch {
      eval %W tag remove sel [%W tag nextrange prompt sel.first sel.last]
      %W mark set insert sel.first
    }
  }

  ## binding editor needed
  ## binding <events> for .tkconrc

  ## <<TkCon_Previous>>
  bind Console <Up> {
    if [%W compare {insert linestart} != {limit linestart}] {
      tkTextSetCursor %W [tkTextUpDownLine %W -1]
    } else {
      if {$tkCon(event) == [tkConEvalSlave history nextid]} {
	set tkCon(cmdbuf) [tkConCmdGet %W]
      }
      if [catch {tkConEvalSlave history event \
	  [incr tkCon(event) -1]} tkCon(tmp)] {
	incr tkCon(event)
      } else {
	%W delete limit end
	%W insert limit $tkCon(tmp)
	%W see end
      }
    }
  }
  ## <<TkCon_Next>>
  bind Console <Down> {
    if [%W compare {insert linestart} != {end-1c linestart}] {
      tkTextSetCursor %W [tkTextUpDownLine %W 1]
    } else {
      if {$tkCon(event) < [tkConEvalSlave history nextid]} {
	%W delete limit end
	if {[incr tkCon(event)] == [tkConEvalSlave history nextid]} {
	  %W insert limit $tkCon(cmdbuf)
	} else {
	  %W insert limit [tkConEvalSlave history event $tkCon(event)]
	}
	%W see end
      }
    }
  }
  ## <<TkCon_ExpandFile>>
  bind Console <Tab> {
    if [%W compare insert > limit] {tkConExpand %W path}
    break
  }
  ## <<TkCon_ExpandProc>>
  bind Console <Control-P> {
    if [%W compare insert > limit] {tkConExpand %W proc}
  }
  ## <<TkCon_ExpandVar>>
  bind Console <Control-V> {
    if [%W compare insert > limit] {tkConExpand %W var}
  }
  ## <<TkCon_Tab>>
  bind Console <Control-i> {
    if [%W compare insert >= limit] {
      tkConInsert %W \t
    }
  }
  ## <<TkCon_Eval>> - no mod
  bind Console <Return> {
    tkConEval %W
  }
  bind Console <KP_Enter> [bind Console <Return>]
  bind Console <Delete> {
    if {[string comp {} [%W tag nextrange sel 1.0 end]] \
	&& [%W compare sel.first >= limit]} {
      %W delete sel.first sel.last
    } elseif {[%W compare insert >= limit]} {
      %W delete insert
      %W see insert
    }
  }
  bind Console <BackSpace> {
    if {[string comp {} [%W tag nextrange sel 1.0 end]] \
	    && [%W compare sel.first >= limit]} {
      %W delete sel.first sel.last
    } elseif {[%W compare insert != 1.0] && [%W compare insert > limit]} {
      %W delete insert-1c
      %W see insert
    }
  }
  bind Console <Control-h> [bind Console <BackSpace>]

  bind Console <KeyPress> {
    tkConInsert %W %A
  }

  bind Console <Control-a> {
    if [%W compare {limit linestart} == {insert linestart}] {
      tkTextSetCursor %W limit
    } else {
      tkTextSetCursor %W {insert linestart}
    }
  }
  bind Console <Control-d> {
    if [%W compare insert < limit] break
    %W delete insert
  }
  bind Console <Control-k> {
    if [%W compare insert < limit] break
    if [%W compare insert == {insert lineend}] {
      %W delete insert
    } else {
      %W delete insert {insert lineend}
    }
  }
  ## <<TkCon_Clear>>
  bind Console <Control-l> {
    ## Clear console buffer, without losing current command line input
    set tkCon(tmp) [tkConCmdGet %W]
    clear
    tkConPrompt {} $tkCon(tmp)
  }
  ## <<TkCon_NextImmediate>>
  bind Console <Control-n> {
    ## Goto next command in history
    if {$tkCon(event) < [tkConEvalSlave history nextid]} {
      %W delete limit end
      if {[incr tkCon(event)] == [tkConEvalSlave history nextid]} {
	%W insert limit $tkCon(cmdbuf)
      } else {
	%W insert limit [tkConEvalSlave history event $tkCon(event)]
      }
      %W see end
    }
  }
  ## <<TkCon_PreviousImmediate>>
  bind Console <Control-p> {
    ## Goto previous command in history
    if {$tkCon(event) == [tkConEvalSlave history nextid]} {
      set tkCon(cmdbuf) [tkConCmdGet %W]
    }
    if [catch {tkConEvalSlave history event \
		   [incr tkCon(event) -1]} tkCon(tmp)] {
      incr tkCon(event)
    } else {
      %W delete limit end
      %W insert limit $tkCon(tmp)
      %W see end
    }
  }
  ## <<TkCon_PreviousSearch>>
  bind Console <Control-r> {
    ## Search history reverse
    if {$tkCon(event) == [tkConEvalSlave history nextid]} {
      set tkCon(cmdbuf) [tkConCmdGet %W]
    } elseif 0 {
      ## FIX
      ## event ids get confusing (to user) when they 'cancel' a history
      ## search.  This should reassign the event id properly.
    }
    set tkCon(tmp1) [string len $tkCon(cmdbuf)]
    incr tkCon(tmp1) -1
    while 1 {
      if {[catch {tkConEvalSlave history event \
		      [incr tkCon(event) -1]} tkCon(tmp)]} {
	incr tkCon(event)
	break
      } elseif {![string comp $tkCon(cmdbuf) \
		      [string range $tkCon(tmp) 0 $tkCon(tmp1)]]} {
	%W delete limit end
	%W insert limit $tkCon(tmp)
	break
      }
    }
    %W see end
  }
  ## <<TkCon_NextSearch>>
  bind Console <Control-s> {
    ## Search history forward
    set tkCon(tmp1) [string len $tkCon(cmdbuf)]
    incr tkCon(tmp1) -1
    while {$tkCon(event) < [tkConEvalSlave history nextid]} {
      if {[incr tkCon(event)] == [tkConEvalSlave history nextid]} {
	%W delete limit end
	%W insert limit $tkCon(cmdbuf)
	break
      } elseif {![catch {tkConEvalSlave history event \
			     $tkCon(event)} tkCon(tmp)]
		&& ![string comp $tkCon(cmdbuf) \
			 [string range $tkCon(tmp) 0 $tkCon(tmp1)]]} {
	%W delete limit end
	%W insert limit $tkCon(tmp)
	break
      }
    }
    %W see end
  }
  ## <<TkCon_Transpose>>
  bind Console <Control-t> {
    ## Transpose current and previous chars
    if [%W compare insert > limit] { tkTextTranspose %W }
  }
  ## <<TkCon_ClearLine>>
  bind Console <Control-u> {
    ## Clear command line (Unix shell staple)
    %W delete limit end
  }
  ## <<TkCon_SaveCommand>>
  bind Console <Control-z> {
    ## Save command buffer (swaps with current command)
    set tkCon(tmp) $tkCon(cmdsave)
    set tkCon(cmdsave) [tkConCmdGet %W]
    if {[string match {} $tkCon(cmdsave)]} {
      set tkCon(cmdsave) $tkCon(tmp)
    } else {
      %W delete limit end-1c
    }
    tkConInsert %W $tkCon(tmp)
    %W see end
  }
  catch {bind Console <Key-Page_Up>   { tkTextScrollPages %W -1 }}
  catch {bind Console <Key-Prior>     { tkTextScrollPages %W -1 }}
  catch {bind Console <Key-Page_Down> { tkTextScrollPages %W 1 }}
  catch {bind Console <Key-Next>      { tkTextScrollPages %W 1 }}
  bind Console <$tkCon(meta)-d> {
    if [%W compare insert >= limit] {
      %W delete insert {insert wordend}
    }
  }
  bind Console <$tkCon(meta)-BackSpace> {
    if [%W compare {insert -1c wordstart} >= limit] {
      %W delete {insert -1c wordstart} insert
    }
  }
  bind Console <$tkCon(meta)-Delete> {
    if [%W compare insert >= limit] {
      %W delete insert {insert wordend}
    }
  }
  bind Console <ButtonRelease-2> {
    if {(!$tkPriv(mouseMoved) || $tk_strictMotif) \
	    && ![catch {selection get -displayof %W} tkCon(tmp)]} {
      if [%W compare @%x,%y < limit] {
	%W insert end $tkCon(tmp)
      } else {
	%W insert @%x,%y $tkCon(tmp)
      }
      if [string match *\n* $tkCon(tmp)] {tkConEval %W}
    }
  }

  ##
  ## End Console bindings
  ##

  ##
  ## Bindings for doing special things based on certain keys
  ##
  bind PostCon <Key-parenright> {
    if {$tkCon(lightbrace) && $tkCon(blinktime)>99 &&
	[string comp \\ [%W get insert-2c]]} {
      tkConMatchPair %W \( \) limit
    }
  }
  bind PostCon <Key-bracketright> {
    if {$tkCon(lightbrace) && $tkCon(blinktime)>99 &&
	[string comp \\ [%W get insert-2c]]} {
      tkConMatchPair %W \[ \] limit
    }
  }
  bind PostCon <Key-braceright> {
    if {$tkCon(lightbrace) && $tkCon(blinktime)>99 &&
	[string comp \\ [%W get insert-2c]]} {
      tkConMatchPair %W \{ \} limit
    }
  }
  bind PostCon <Key-quotedbl> {
    if {$tkCon(lightbrace) && $tkCon(blinktime)>99 &&
	[string comp \\ [%W get insert-2c]]} {
      tkConMatchQuote %W limit
    }
  }

  bind PostCon <KeyPress> {
    if {$tkCon(lightcmd) && [string comp {} %A]} { tkConTagProc %W }
  }
}

## tkConTagProc - tags a procedure in the console if it's recognized
## This procedure is not perfect.  However, making it perfect wastes
## too much CPU time...  Also it should check the existence of a command
## in whatever is the connected slave, not the master interpreter.
##
proc tkConTagProc w {
  set i [$w index "insert-1c wordstart"]
  set j [$w index "insert-1c wordend"]
  if {[string comp {} \
	   [tkConEvalAttached info command [list [$w get $i $j]]]]} {
    $w tag add proc $i $j
  } else {
    $w tag remove proc $i $j
  }
}

## tkConMatchPair - blinks a matching pair of characters
## c2 is assumed to be at the text index 'insert'.
## This proc is really loopy and took me an hour to figure out given
## all possible combinations with escaping except for escaped \'s.
## It doesn't take into account possible commenting... Oh well.  If
## anyone has something better, I'd like to see/use it.  This is really
## only efficient for small contexts.
# ARGS:	w	- console text widget
# 	c1	- first char of pair
# 	c2	- second char of pair
# Calls:	tkConBlink
## 
proc tkConMatchPair {w c1 c2 {lim 1.0}} {
  if [string comp {} [set ix [$w search -back $c1 insert $lim]]] {
    while {[string match {\\} [$w get $ix-1c]] &&
	   [string comp {} [set ix [$w search -back $c1 $ix-1c $lim]]]} {}
    set i1 insert-1c
    while {[string comp {} $ix]} {
      set i0 $ix
      set j 0
      while {[string comp {} [set i0 [$w search $c2 $i0 $i1]]]} {
	append i0 +1c
	if {[string match {\\} [$w get $i0-2c]]} continue
	incr j
      }
      if {!$j} break
      set i1 $ix
      while {$j && [string comp {} [set ix [$w search -back $c1 $ix $lim]]]} {
	if {[string match {\\} [$w get $ix-1c]]} continue
	incr j -1
      }
    }
    if [string match {} $ix] { set ix [$w index $lim] }
  } else { set ix [$w index $lim] }
  tkConBlink $w $ix [$w index insert]
}

## tkConMatchQuote - blinks between matching quotes.
## Blinks just the quote if it's unmatched, otherwise blinks quoted string
## The quote to match is assumed to be at the text index 'insert'.
# ARGS:	w	- console text widget
# Calls:	tkConBlink
## 
proc tkConMatchQuote {w {lim 1.0}} {
  set i insert-1c
  set j 0
  while {[string comp {} [set i [$w search -back \" $i $lim]]]} {
    if {[string match {\\} [$w get $i-1c]]} continue
    if {!$j} {set i0 $i}
    incr j
  }
  if [expr $j%2] {
    tkConBlink $w $i0 [$w index insert]
  } else {
    tkConBlink $w [$w index insert-1c] [$w index insert]
  }
}

## tkConBlink - blinks between 2 indices for a specified duration.
# ARGS:	w	- console text widget
# 	i1	- start index to blink region
# 	i2	- end index of blink region
# 	dur	- duration in usecs to blink for
# Outputs:	blinks selected characters in $w
## 
proc tkConBlink {w i1 i2} {
  global tkCon
  $w tag add blink $i1 $i2
  after $tkCon(blinktime) $w tag remove blink $i1 $i2
  return
}


## tkConInsert
## Insert a string into a text console at the point of the insertion cursor.
## If there is a selection in the text, and it covers the point of the
## insertion cursor, then delete the selection before inserting.
# ARGS:	w	- text window in which to insert the string
# 	s	- string to insert (usually just a single char)
# Outputs:	$s to text widget
## 
proc tkConInsert {w s} {
  if {[string match {} $s] || [string match disabled [$w cget -state]]} {
    return
  }
  if [$w comp insert < limit] {
    $w mark set insert end
  }
  catch {
    if {[$w comp sel.first <= insert] && [$w comp sel.last >= insert]} {
      $w delete sel.first sel.last
    }
  }
  $w insert insert $s
  $w see insert
}

## tkConExpand - 
# ARGS:	w	- text widget in which to expand str
# 	type	- type of expansion (path / proc / variable)
# Calls:	tkConExpand(Pathname|Procname|Variable)
# Outputs:	The string to match is expanded to the longest possible match.
#		If tkCon(showmultiple) is non-zero and the user longest match
#		equaled the string to expand, then all possible matches are
#		output to stdout.  Triggers bell if no matches are found.
# Returns:	number of matches found
## 
proc tkConExpand {w type} {
  set exp "\[^\\]\[ \t\n\r\[\{\"\$]"
  set tmp [$w search -back -regexp $exp insert-1c limit-1c]
  if [string compare {} $tmp] {append tmp +2c} else {set tmp limit}
  if [$w compare $tmp >= insert] return
  set str [$w get $tmp insert]
  switch -glob $type {
    pa* { set res [tkConExpandPathname $str] }
    pr* { set res [tkConExpandProcname $str] }
    v*  { set res [tkConExpandVariable $str] }
    default {set res {}}
  }
  set len [llength $res]
  if $len {
    $w delete $tmp insert
    $w insert $tmp [lindex $res 0]
    if {$len > 1} {
      global tkCon
      if {$tkCon(showmultiple) && ![string comp [lindex $res 0] $str]} {
	puts stdout [lreplace $res 0 0]
      }
    }
  } else bell
  return [incr len -1]
}

## tkConExpandPathname - expand a file pathname based on $str
## This is based on UNIX file name conventions
# ARGS:	str	- partial file pathname to expand
# Calls:	tkConExpandBestMatch
# Returns:	list containing longest unique match followed by all the
#		possible further matches
## 
proc tkConExpandPathname str {
  set pwd [tkConEvalAttached pwd]
  if [catch {tkConEvalAttached [list cd [file dirname $str]]} err] {
    return -code error $err
  }
  if [catch {lsort [tkConEvalAttached glob [file tail $str]*]} m] {
    set match {}
  } else {
    if {[llength $m] > 1} {
      set tmp [tkConExpandBestMatch $m [file tail $str]]
      if [string match ?*/* $str] {
	set tmp [file dirname $str]/$tmp
      } elseif {[string match /* $str]} {
	set tmp /$tmp
      }
      regsub -all { } $tmp {\\ } tmp
      set match [linsert $m 0 $tmp]
    } else {
      ## This may look goofy, but it handles spaces in path names
      eval append match $m
      if [file isdir $match] {append match /}
      if [string match ?*/* $str] {
	set match [file dirname $str]/$match
      } elseif {[string match /* $str]} {
	set match /$match
      }
      regsub -all { } $match {\\ } match
      ## Why is this one needed and the ones below aren't!!
      set match [list $match]
    }
  }
  tkConEvalAttached [list cd $pwd]
  return $match
}

## tkConExpandProcname - expand a tcl proc name based on $str
# ARGS:	str	- partial proc name to expand
# Calls:	tkConExpandBestMatch
# Returns:	list containing longest unique match followed by all the
#		possible further matches
## 
proc tkConExpandProcname str {
  set match [tkConEvalAttached info commands $str*]
  if {[llength $match] > 1} {
    regsub -all { } [tkConExpandBestMatch $match $str] {\\ } str
    set match [linsert $match 0 $str]
  } else {
    regsub -all { } $match {\\ } match
  }
  return $match
}

## tkConExpandVariable - expand a tcl variable name based on $str
# ARGS:	str	- partial tcl var name to expand
# Calls:	tkConExpandBestMatch
# Returns:	list containing longest unique match followed by all the
#		possible further matches
## 
proc tkConExpandVariable str {
  if [regexp {([^\(]*)\((.*)} $str junk ary str] {
    ## Looks like they're trying to expand an array.
    set match [tkConEvalAttached array names $ary $str*]
    if {[llength $match] > 1} {
      set vars $ary\([tkConExpandBestMatch $match $str]
      foreach var $match {lappend vars $ary\($var\)}
      return $vars
    } else {set match $ary\($match\)}
    ## Space transformation avoided for array names.
  } else {
    set match [tkConEvalAttached info vars $str*]
    if {[llength $match] > 1} {
      regsub -all { } [tkConExpandBestMatch $match $str] {\\ } str
      set match [linsert $match 0 $str]
    } else {
      regsub -all { } $match {\\ } match
    }
  }
  return $match
}

## tkConExpandBestMatch2 - finds the best unique match in a list of names
## Improves upon the speed of the below proc only when $l is small
## or $e is {}.  $e is extra for compatibility with proc below.
# ARGS:	l	- list to find best unique match in
# Returns:	longest unique match in the list
## 
proc tkConExpandBestMatch2 {l {e {}}} {
  set s [lindex $l 0]
  if {[llength $l]>1} {
    set i [expr [string length $s]-1]
    foreach l $l {
      while {$i>=0 && [string first $s $l]} {
	set s [string range $s 0 [incr i -1]]
      }
    }
  }
  return $s
}

## tkConExpandBestMatch - finds the best unique match in a list of names
## The extra $e in this argument allows us to limit the innermost loop a
## little further.  This improves speed as $l becomes large or $e becomes long.
# ARGS:	l	- list to find best unique match in
# 	e	- currently best known unique match
# Returns:	longest unique match in the list
## 
proc tkConExpandBestMatch {l {e {}}} {
  set ec [lindex $l 0]
  if {[llength $l]>1} {
    set e  [string length $e]; incr e -1
    set ei [string length $ec]; incr ei -1
    foreach l $l {
      while {$ei>=$e && [string first $ec $l]} {
	set ec [string range $ec 0 [incr ei -1]]
      }
    }
  }
  return $ec
}

# Here is a group of functions that is only used when Tkcon is
# executed in a safe interpreter. It provides safe versions of
# missing functions. For example:
#
# - "tk appname" returns "tkcon.tcl" but cannot be set
# - "toplevel" is equivalent to "frame", only it is automatically
#   packed.
# - The "source", "load", "open", "file" and "exit" functions are
#   mapped to corresponding functions in the parent interpreter.
#
# Further on, Tk cannot be really loaded. Still the safe "load"
# provedes a speciall case. The Tk can be divided into 4 groups,
# that each has a safe handling procedure.
#
# - "tkConSafeItem" handles commands like "button", "canvas" ......
#   Each of these functions has the window name as first argument.
# - "tkConSafeManage" handles commands like "pack", "place", "grid",
#   "winfo", which can have multiple window names as arguments.
# - "tkConSafeWindow" handles all windows, such as ".". For every
#   window created, a new alias is formed which also is handled by
#   this function.
# - Other (e.g. bind, bindtag, image), which need their own function.
#
## These functions courtesy Jan Nijtmans (nijtmans@nici.kun.nl)
##
if {[string compare [info command tk] tk]} {
  proc tk {option args} {
    if {![string match app* $option]} {
      error "wrong option \"$option\": should be appname"
    }
    return "tkcon.tcl"
  }
}
 
if {[string compare [info command toplevel] toplevel]} {
  proc toplevel {name args} {
    eval frame $name $args
    pack $name
  }
}

proc tkConSafeSource {i f} {
  set fd [open $f r]
  set r [read $fd]
  close $fd
  if {[catch {interp eval $i $r} msg]} {
    error $msg
  }
}

proc tkConSafeOpen {i f {m r}} {
    set fd [open $f $m]
    interp transfer {} $fd $i
    return $fd
}

proc tkConSafeLoad {i f p} {
  global tk_version tk_patchLevel tk_library
  if [string compare $p Tk] {
    load $f $p $i
  } else {
    foreach command {button canvas checkbutton entry frame label
      listbox message radiobutton scale scrollbar text toplevel} {
      $i alias $command tkConSafeItem $i $command
    }
    $i alias image tkConSafeImage $i
    foreach command {pack place grid destroy winfo} {
      $i alias $command tkConSafeManage $i $command
    }
    if [string comp {} [info command event]] {
      $i alias event tkConSafeManage $i $command
    }
    frame .${i}_dot -width 300 -height 300 -relief raised
    pack .${i}_dot -side left
    $i alias tk tk
    $i alias bind tkConSafeBind $i
    $i alias bindtags tkConSafeBindtags $i
    $i alias . tkConSafeWindow $i {}
    foreach var {tk_version tk_patchLevel tk_library} {
      $i eval set $var [set $var]
    }
    $i eval {
      package provide Tk $tk_version
      if {[lsearch -exact $auto_path $tk_library] < 0} {
	lappend auto_path $tk_library
      }
    }
    return ""
  }
}

proc tkConSafeSubst {i a} {
  set arg1 ""
  foreach {arg value} $a {
    if {![string compare $arg -textvariable] ||
      ![string compare $arg -variable]} {
      set newvalue "[list $i] $value"
      global $newvalue
      if [interp eval $i info exists $value] {
      set $newvalue [interp eval $i set $value]
      } else {
      catch {unset $newvalue}
      }
      $i eval trace variable $value rwu \{[list tkcon set $newvalue $i]\}
      set value $newvalue
    } elseif {![string compare $arg -command]} {
      set value [list $i eval $value]
    }
    lappend arg1 $arg $value
  }
  return $arg1
}

proc tkConSafeItem {i command w args} {
  set args [tkConSafeSubst $i $args]
  set code [catch "$command [list .${i}_dot$w] $args" msg]
  $i alias $w tkConSafeWindow $i $w
  regsub -all .${i}_dot $msg {} msg
  return -code $code $msg
}

proc tkConSafeManage {i command args} {
  set args1 ""
  foreach arg $args {
    if [string match . $arg] {
      set arg .${i}_dot
    } elseif [string match .* $arg] {
      set arg ".${i}_dot$arg"
    }
    lappend args1 $arg
  }
  set code [catch "$command $args1" msg]
  regsub -all .${i}_dot $msg {} msg
  return -code $code $msg
}

#
# FIX: this funcion doesn't work yet if the binding starts with "+".
#
proc tkConSafeBind {i w args} {
  if [string match . $w] {
    set w .${i}_dot
  } elseif [string match .* $w] {
    set w ".${i}_dot$w"
  }
  if {[llength $args] > 1} {
    set args [list [lindex $args 0] "[list $i] eval [list [lindex $args 1]]"]
  }
  set code [catch "bind $w $args" msg]
  if {[llength $args] <2 && code == 0} {
    set msg [lindex $msg 3]
  }
  return -code $code $msg
}

proc tkConSafeImage {i option args} {
  set code [catch "image $option $args" msg]
  if {[string match cr* $option]} {
    $i alias $msg $msg
  }
  return -code $code $msg
}

proc tkConSafeBindtags {i w {tags {}}} {
  if [string match . $w] {
    set w .${i}_dot
  } elseif [string match .* $w] {
    set w ".${i}_dot$w"
  }
  set newtags {}
  foreach tag $tags {
    if [string match . $tag] {
      lappend newtags .${i}_dot
    } elseif [string match .* $tag] {
      lappend newtags ".${i}_dot$tag"
    } else {
      lappend newtags $tag
    }
  }
  if [string match $tags {}] {
    set code [catch {bindtags $w} msg]
    regsub -all \\.${i}_dot $msg {} msg
  } else {
    set code [catch {bindtags $w $newtags} msg]
  }
  return -code $code $msg
}

proc tkConSafeWindow {i w option args} {
  if {[string match conf* $option] && [llength $args] > 1} {
    set args [tkConSafeSubst $i $args]
  } elseif {[string match itemco* $option] && [llength $args] > 2} {
    set args "[list [lindex $args 0]] [tkConSafeSubst $i [lrange $args 1 end]]"
  } elseif {[string match cr* $option]} {
    if {[llength $args]%2} {
      set args "[list [lindex $args 0]] [tkConSafeSubst $i [lrange $args 1 end]]"
    } else {
      set args [tkConSafeSubst $i $args]
    }
  } elseif {[string match bi* $option] && [llength $args] > 2} {
    set args [list [lindex $args 0] [lindex $args 1] "[list $i] eval [list [lindex $args 2]]"]
  }
  set code [catch ".${i}_dot$w $option $args" msg]
  if {$code} {
    regsub -all .${i}_dot $msg {} msg
  } elseif {[string match conf* $option] || [string match itemco* $option]} {
    if {[llength $args] == 1} {
      switch -- $args {
      -textvariable - -variable {
        set msg "[lrange $msg 0 3] [list [lrange [lindex $msg 4] 1 end]]"
      }
      -command - updatecommand {
        set msg "[lrange $msg 0 3] [list [lindex [lindex $msg 4] 2]]"
      }
      }
    } elseif {[llength $args] == 0} {
      set args1 ""
      foreach el $msg {
      switch -- [lindex $el 0] {
        -textvariable - -variable {
          set el "[lrange $el 0 3] [list [lrange [lindex $el 4] 1 end]]"
        }
        -command - updatecommand {
          set el "[lrange $el 0 3] [list [lindex [lindex $el 4] 2]]"
        }
      }
      lappend args1 $el
      }
      set msg $args1
    }
  } elseif {[string match cg* $option] || [string match itemcg* $option]} {
    switch -- $args {
      -textvariable - -variable {
        set msg [lrange $msg 1 end]
      }
      -command - updatecommand {
      set msg [lindex $msg 2]
      }
    }
  } elseif [string match bi* $option] {
    if {[llength $args] == 2 && $code == 0} {
      set msg [lindex $msg 2]
    }
  }
  return -code $code $msg
}

## tkConResource - re'source's this script into current console
## Meant primarily for my development of this program.  It follows
## links until the ultimate source is found.
## 
set tkCon(SCRIPT) [info script]
if !$tkCon(WWW) {
  while {[string match link [file type $tkCon(SCRIPT)]]} {
    set link [file readlink $tkCon(SCRIPT)]
    if [string match relative [file pathtype $link]] {
      set tkCon(SCRIPT) [file join [file dirname $tkCon(SCRIPT)] $link]
    } else {
      set tkCon(SCRIPT) $link
    }
  }
  catch {unset link}
  if [string match relative [file pathtype $tkCon(SCRIPT)]] {
    set tkCon(SCRIPT) [file join [pwd] $tkCon(SCRIPT)]
  }
}
proc tkConResource {} {
  global tkCon
  uplevel \#0 [list source $tkCon(SCRIPT)]
  tkConBindings
  tkConInitSlave $tkCon(exec)
}

## Initialize only if we haven't yet
##
if [catch {winfo exists $tkCon(root)}] tkConInit
