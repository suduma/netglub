##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "SquirrelMail" do
author "Andrew Horton"
version "0.1"
description "Opensource Webmail written in PHP. Homepage: http://squirrelmail.org"

# tends to get customised

matches [
{:name=>"default title", 
:probability=>100,
:text=>"<title>SquirrelMail - Login</title>"},

{:name=>"js function", 
:probability=>100,
:text=>"function squirrelmail_loginpage_onload()"},

{:name=>"css comment", 
:probability=>100,
:text=>'/* avoid stupid IE6 bug with frames and scrollbars */'},

{:name=>"login",
:probability=>100,
:text=>'<b>SquirrelMail Login</b>'}

]

# <SMALL>SquirrelMail version 1.2.7<BR>
# <small>SquirrelMail version 1.4.4<
# <small>SquirrelMail version 1.4.9a<br />
def passive
	m=[]

	#SquirrelMail version 1.4.4<
	if @body =~ /<(small|SMALL)>SquirrelMail version ([^<]+)</
		version=@body.scan(/<(small|SMALL)>SquirrelMail version ([^<]+)</)[0][1]
		m << {:probability=>100,:name=>"version",:version=>version} 
	end
	
	# Set-Cookie: SQMSESSID=5055d75dbcac067f72b509effc42cc50; path=/
	m << {:name=>"SQMSESSID cookie", :probability=>100 } if @meta["set-cookie"] =~ /SQMSESSID=/i		
	m
end


end

