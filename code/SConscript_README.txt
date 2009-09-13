These SConscripts are for use with Xreal svn SConstruct.
When I built them I found that I had to comment out two of the SConscript files, and build them one at a time.
I think this is due to the SConscripts using '#weaver/code/*.c' as paths to files instead of 'weaver/code/*.c'
I'm pretty sure the # is like root. I did this for the SConscripts to be able to reference files correctly.
Apparently doing this from multiple environments (SConscripts) at once doesn't work.
I know its crap, but I don't use SCons so I'm not pressed to fix it.
If you work out the correct way of doing this, tell DerSaidin. Thanks.

Add the following at the appropriate locations to Xreal svn SConstruct:

        opts.Add(BoolVariable('weaver', 'Set to 1 to compile the Weaver game', 0))

        if env['weaver'] == 1:
        	SConscript('weaver/code/SConscript_weaver_cgame', build_dir='build/weaver/cgame', duplicate=0)
        #	SConscript('weaver/code/SConscript_weaver_game', build_dir='build/weaver/game', duplicate=0)
        #	SConscript('weaver/code/SConscript_weaver_ui', build_dir='build/weaver/ui', duplicate=0)

Example (after adding following):
$ cd xreal
$ scons weaver=1 arch=linux-x86_64
$ vim SConstruct
$ scons weaver=1 arch=linux-x86_64
$ vim SConstruct
$ scons weaver=1 arch=linux-x86_64

At each of the `vim SConstruct` steps, swap the #s around so the next line is not commented but the other two are.
So after the first one it becomes:

        if env['weaver'] == 1:
        #	SConscript('weaver/code/SConscript_weaver_cgame', build_dir='build/weaver/cgame', duplicate=0)
        	SConscript('weaver/code/SConscript_weaver_game', build_dir='build/weaver/game', duplicate=0)
        #	SConscript('weaver/code/SConscript_weaver_ui', build_dir='build/weaver/ui', duplicate=0)
        
Then after second edit:

        if env['weaver'] == 1:
        #	SConscript('weaver/code/SConscript_weaver_cgame', build_dir='build/weaver/cgame', duplicate=0)
        #	SConscript('weaver/code/SConscript_weaver_game', build_dir='build/weaver/game', duplicate=0)
        	SConscript('weaver/code/SConscript_weaver_ui', build_dir='build/weaver/ui', duplicate=0)
            
            
The other option is to copy these files up 2 levels to where all the other ones are then do:
:%s/'#weaver/'weaver/g
Then add hashes back in for all the ones after the 
if env['arch'] ==
Have a look at existing xreal SConscripts to see which lines should still be '#weaver'
