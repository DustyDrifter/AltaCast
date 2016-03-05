# AltaCast

AltaCast is a continuation of the Oddcast/Edcast streaming software that can stream to either Icecast or SHOUTcast servers.

List of Changes/Improvements:

    -Made main window slightly taller to accommodate larger encoder window
    
    -Items in main window spread out a little
    
    -Configuration window now uses tabs like Shoutcast encoder instead of buttons (looks more professional).
    
    -YP configuration is editable when the “Public” setting is disabled.
    
    -The “OK” and “Cancel” buttons were moved to the bottom on either side to make it less likely to click the wrong one
    
    -Server type drop down box enlarged
    
    -Encoder Password field now hides the password
    
    -Cleaned up the code to make it more compliant with today’s standards
    
    -Tested working and stable on Windows XP, Server 2003 and Windows 7

Bugs and other issues:

Deleting multiple encoders in the main window one at a time from the bottom up will crash it. As a workaround, I made the encoder window so that you can select multiple encoders. It will still only delete one at a time, but selecting multiple encoders anyway keeps it from crashing.
