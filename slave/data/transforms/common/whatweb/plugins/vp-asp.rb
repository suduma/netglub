##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "VP-ASP" do
author "Andrew Horton"
version "0.1"

description "VP-ASP (Virtual Programming - ASP) Shopping Cart. Free & commercial versions. http://johnny.ihackstuff.com/ghdb/?function=detail&id=324  Homepage:www.vpasp.com"

examples %w|higginswoodfloors.com/shopping www.andrewbrownhomedirect.com/default.asp www.moremagictricks.co.uk www.stonebridge.com www.valuevision.com.ph/shopdisplayproducts.asp www.partysballoons.com.au www.palouseriver.net |

matches [
{:name=>"GHDB: filetype:asp inurl:shopdisplayproducts.asp",
:probability=>75,
:ghdb=>'filetype:asp inurl:"shopdisplayproducts.asp"'},

{:name=>"vs350.js",
:probability=>75,
:regexp=>/src="vs350.js"/},

{:name=>"powered by link 1",
:probability=>100,
:regexp=>/<a href="http:\/\/www.vpasp.com">Shopping Cart<\/a> powered by VP-ASP<\/p>/},

{:name=>"powered by link 2",
:probability=>100,
:regexp=>/<a href="http:\/\/www.vpasp.com">Powered By VP-ASP Shopping Cart<\/a>/},

{:name=>"shopdisplayproducts.asp",
:probability=>75,
:regexp=>/shopdisplayproducts.asp?id=/}
]

# <title>VP-ASP Shopping Cart 6.00</title>
# <title>VP-ASP Shopping Cart 4.50a</title>
# <title>VP-ASP Shopping Cart 6.50</title>
# <a href="http://www.vpasp.com">Powered By VP-ASP Shopping Cart</a>

def passive
	m=[]
	if @body =~ /<title>VP-ASP Shopping Cart [^<]*<\/title>/
		version=@body.scan(/<title>VP-ASP Shopping Cart ([^ <]*)/)[0][0]
		m << {:probability=>100,:name=>"powered by title",:version=>version}
	end
	m
end

end

