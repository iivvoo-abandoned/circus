#
# The only two commands implemented are:
#
# raw - sends a raw message to the server
# bindcmd - binds a tcl command to a circus /command

proc privmsg {nick msg} {
	raw "PRIVMSG $nick :$msg"
}

proc msg {nick msg} {
#
# if nick starts with =, use dcc chat, else use normal privmsg

    if {[ string first "=" $nick ] == -1} then {
        privmsg $nick $msg
    } else {
        dcc msg [ string trimleft $nick "=" ] $msg
    }
}

proc xmas {nick} {
msg $nick  {02,02                         08__/\\__                      }
msg $nick  {02,02                         08\\ ** /                      }
msg $nick  {02,02                         08/_**_\\                      }
msg $nick  {02,02                         09,03/ 08\\/09,03 \\2,2                      }
msg $nick  {02,02                       09,03/3,9/ ~  $$ \\9,3\\2,2                    }
msg $nick  {02,02                     09,03/___3,9______9,3___\\2,2                  }
msg $nick  {02,02                    09,03/3,9/  *  %    \\9,3\\8,2~                  }
msg $nick  {0,0                  ~09,03/ 3,9/  @   +  #  \\9,3 \\0,0                }
msg $nick  {0,0                 09,03/__3,9/______________\\9,3__\\8,0~             }
msg $nick  {0,0                * 09,03/  3,9/   +        \\9,3  \\8,0 *             }
msg $nick  {0,0               09,03/%  3,9/$$ 4M E R R Y3 * \\9,3 # \\0,0              }
msg $nick  {0,0             ~09,03/____3,9/____o___________\\9,3____\\0,0           }
msg $nick  {0,0             * 09,03/  ^3,9/            @  \\9,3    \\8,0 *          }
msg $nick  {0,0             09,03/ %  3,9/4C H R I S T M A S\\9,3   # \\0,0          }
msg $nick  {0,0          09,03o/~    3,9/o          #   %   \\9,3o    ~\\0,0        }
msg $nick  {0,0        09,03/_____3,9/_______________________\\9,3______\\0,0       }
msg $nick  {0,0        2*   4o   2*    4o   1,5| 2* 1|0,0  4o   2*    4o   2*       }
msg $nick  {0,0                         1,5|   |1,0                       }
msg $nick  {0,0                         1,5|___|1,0                       }
msg $nick  {}
msg $nick  {      The 2c3I4R5C6u7s development team whishes you a}
msg $nick  {                    Merry Christmas}
msg $nick  {                         and a}
msg $nick  {                    Happy New Year!}
}

#
# Now bind our tcl command to a circus command (one that you can enter in
# an input entry) using the bindcmd command.

#
# Without a second argument, this will simply bind the xmas-proc to /xmas.
# An optional second parameter defines the /cmd name

bindcmd xmas
