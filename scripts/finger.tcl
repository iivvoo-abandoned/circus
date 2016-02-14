#
# The only two commands implemented are:
#
# raw - sends a raw message to the server
# bindcmd - binds a tcl command to a circus /command
#
# Using these two, define some misc. other commands.

#
#
# How to use: type /load finger.tcl in circus, start a dcc session with
# a user (/dcc chat somenick, or use the query-window), and type:
#
# /finger =somenick
# /circus =somenick
# /okay =somenick

#
# WARNING: Don't use this script without a dcc chat!! It will flood you
# of the server! :)

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

proc finger {nick} {
msg $nick {01                       zc               }
msg $nick {08                     d$"$               }
msg $nick {09                    d%  $               }
msg $nick {03                    $   *F              }
msg $nick {02                    $   ^$              }
msg $nick {12                    *.   *b             }
msg $nick {10                     $    "$            }
msg $nick {06                     '$    ^$.          }
msg $nick {05                      $L     $r         }
msg $nick {07                      4$      *r        }
msg $nick {08                       $       *.       }
msg $nick {04                 .e$$$$$        $.      }
msg $nick {13               .P"               *c.... }
msg $nick {04              .F    .ee.            ""$ }
msg $nick {08              4   $$"  ^*c            4F}
msg $nick {07              ^b        .$L            $}
msg $nick {05               $b    .e$*$c            $}
msg $nick {06               4F*$$$"    "$           $}
msg $nick {10               4.       z$$E           $}
msg $nick {12                $beeee$P"  *b         4F}
msg $nick {02                 $        z$P"     ...$ }
msg $nick {03                 *c    .dP"*b    .$"""" }
msg $nick {09                  ^"$**"    $b  zP      }
msg $nick {08                    $     e$* .$"       }
msg $nick {01                     *$$$*$$$$"         }
}

proc circus {nick} {
msg $nick {0,1    ::::::  ::: ::::::    ::::::  :::  :::  :::::   }
msg $nick {8,1   :+:  :+: :+: :+: :+:  :+:  :+: :+:  :+: :+: :+:  }
msg $nick {7,1  +:+       +:+ +:+ +:+ +:+       +:+  +:+  +:+     }
msg $nick {4,1  +#+       +#+ +#++#+  +#+       +#+  +#+    +#+   }
msg $nick {6,1   #+#  #+# #+# #+# #+#  #+#  #+#  #+##+#  #+# #+#  }
msg $nick {2,1    ######  ### ###  ###  ######    ####    #####   }
} 

proc okay {nick} {
finger $nick
msg $nick { }
circus $nick
}

#
# Make the new procedures visible as circus command:

bindcmd finger
bindcmd circus
bindcmd okay
