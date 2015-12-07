##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Mailto" do
author "Andrew Horton"
version "0.1"
description "email addresses in mailto: links"
# identifying strings
# mailto: email@address

def passive
	m=[]
	emails=@body.scan(/mailto:([^\"]+)/)
	if emails.size > 0
		match=true
		emails.flatten!
		emails.map! {|e| CGI.unescapeHTML(e)  }
		emails.map! {|e| CGI.unescape(e)  }

		m << {:probability=>100,:name=>"mailto:",:emails=>emails} \
	end

	m		
end

end




