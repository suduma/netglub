##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Moodle" do
author "Andrew Horton"
version "0.1"
description "Educational. Homepage: www.moodle.org"
# identifying strings
# <a title="Moodle 1.9.4+ (Build: 20090415)" href="http://moodle.org/">
# <img style="width:100px;height:30px" src="pix/moodlelogo.gif" alt="moodlelogo" />

matches [
{:name=>"moodle version link",
:probability=>100,
:regexp=>/<a title="Moodle[^"]+" href="http:\/\/moodle.org\/">/},

{:name=>"moodle logo.gif",
:probability=>75,
:regexp=>/<img style="width:100px;height:30px" src="[^"]+\/moodlelogo.gif" alt="moodlelogo" \/>/}
]

# Set-Cookie: MoodleSession=i5ec8g9bnm04b8000jia350n06; path=/

def passive
  m=[]
  m << {:name=>"MoodleSession Cookie", :probability=>100 } if @meta["set-cookie"] =~ /MoodleSession=.*/
  m
end

end

