##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "SiemensSpeedStreamRouter" do
author "Andrew Horton"
version "0.1"
# identifying strings
# <TITLE>SpeedStream Router Management Interface</TITLE>
# <!-- Copyright(C) 2003 Siemens Subscriber Networks -->
# <FRAME SRC="pflogin.htm" NAME="rightFrame"

matches [
{:name=>"intitle:SpeedStream Router Management Interface",
:probability=>100,
:regexp=>/<TITLE>SpeedStream Router Management Interface<\/TITLE>/},

{:name=>"copyright siemens subscriber networks",
:probability=>50,
:regexp=>/<!-- Copyright(C) [0-9]+ Siemens Subscriber Networks -->/},

{:name=>"frame src=pflogin.htm",
:probability=>75,
:regexp=>/<FRAME SRC="pflogin.htm" NAME="rightFrame"/}
]


end

