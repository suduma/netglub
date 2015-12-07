##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "RoundCube" do
author "Andrew Horton"
version "0.1"
description "Opensource Webmail written in PHP. Homepage: http://roundcube.net/"

# Version detection notes: older versions don't have the anti-CSRF token

matches [
{:name=>"default title<title>RoundCube Webmail :: Welcome to RoundCube Webmail</title>", 
:probability=>100,
:text=>"<title>RoundCube Webmail :: Welcome to RoundCube Webmail</title>"},

{:name=>"javascript",
:probability=>100,
:text=>"var rcmail = new rcube_webmail();"},

{:name=>"rcmloginuser",
:probability=>100,
:text=>'<input name="_user" id="rcmloginuser"'},

{:name=>"javascript",
:probability=>100,
:text=>'$(document).ready(function(){ rcmail.init(); });'}

]

# Set-Cookie: roundcube_sessid=573c9cce97436b17e4ab02d83747efb2; path=/
def passive
	m=[]
	m << {:name=>"roundcube_sessid cookie", :probability=>100 } if @meta["set-cookie"] =~ /roundcube_sessid/i		
	m
end


end

