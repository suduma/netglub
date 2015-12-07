##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "MovableType" do
author "Andrew Horton"
version "0.1"
description "Blogging platform  www.movabletype.org"

# /mt/mt-tags.fcgi
# /mt-tb.fcgi
# /mt-cp.[f]?cgi
# /mt-search.cgi
# /mt-user-login.cgi
# /mt-recommend.cgi
#
# <meta name="generator" content="Movable Type Pro 4.23-en" />
# <meta name="generator" content="Movable Type Pro 4.24-en" />
# <meta name="generator" content="Movable Type Enterprise 1.52-en-voltron-r47459-20070213" />
# <meta name="generator" content="Movable Type 3.2" />
# <meta name="generator" content="http://www.movabletype.org/" /> seen on Movable Type 3.34

# <script type="text/javascript" src="http://awearnessblog.com/mt.js"></script>

# <a href="http://sixapart.com">Powered by Movable Type</a>

#/cgi-bin/mt4/plugins/openid-server/server.cgi
#
# example sites:
# www.movabletype.com/showcase/
#
# can't detect tests: boeingblogs.com.randy , .electricartists.com.corporate, filminfocus.com, muledesign.com, www.radaronline.com, 
# www.theatlantic.com, www.thehuffingtonpost.com, www.plasticmind.com
#

# mt-site.js

# to detect for sure
# try /mt, /mt/mt-check.cgi (discloses versions, paths)

matches [
{:name=>"javascript with 'mt' in the name",
:probability=>50,
:regexp=>/<script type="text\/javascript" src="[^"]+mt[-site]?.js"><\/script>/},

{:name=>"mt-tags|mt-tb|mt-cp|mt-search|mt-user-login|mt-recommend cgi",
:probability=>75,
:regexp=>/"[^"]+\/(mt-tags|mt-tb|mt-cp|mt-search|mt-user-login|mt-recommend)\.[f]?cgi[^"]*"/},

{:name=>"meta generator tag",
:probability=>100,
:regexp=>/<meta name="generator" content="http:\/\/www.movabletype.org\/" \/>/},

{:name=>"Powered by link",
:probability=>100,
:regexp=>/<a href="http:\/\/sixapart.com">Powered by Movable Type<\/a>/}

]


def passive
	m=[]
	
	if @body =~ /<meta name="generator" content="Movable Type [^"]*/
		match=true
#		res=@body.scan(/<meta name="generator" content="Movable Type (Pro|Enterprise)?[ ]?([0-9\.]+)(-)?([a-z]+)?([^\"])*" \/>/)[0]
		version=@body.scan(/<meta name="generator" content="Movable Type ([^"]*)/)[0][0]

		m << {:probability=>100,:name=>"meta generator tag", :version=>version} 
		#m << {:probability=>100,:name=>"meta generator tag",:type=>res[0], :version=>res[1], :lang=>res[3]} 
	end	
	m
	
end

end

