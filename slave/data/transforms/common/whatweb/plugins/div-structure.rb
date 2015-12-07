##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Div-Span-Structure" do
author "Andrew Horton"
version "0.1"
description "Fuzzy pattern matching. Analyze the structure of <div> and <span> tags. Turns the opening and closing tag order into a signature. Threshold is set at at least 12 open or closing tags. This can be used to group websites created with something unexpected. The signature is hashed to save log file size. In future, unhashed signatures could be used to group by similarity."

# div & span has found nothing yet
# this produces nothing useful for just div or just span

# inspired by this work - http://www.aharef.info/static/htmlgraph/?url=http://www.morningstarsecurity.com/


# a method to keep the pattern, yet encode to reduce size would be..
# convert binary to decimal
# convert number to hex 


def passive

	# the minimum number of div or span tags we're looking for is 12. this is the noise threshold
	if @body.scan(/(<[\/]?(div|span))[^>]*>/i).flatten.size > 12
		binary=@body.scan(/(<[\/]?(div|span))[^>]*>/i).flatten.map {|x|
			x.downcase!		
			ret='d' if x[1..1] == '/' and x[2..2] == "d"
			ret='D' if x[1..1] == 'd'
			ret='s' if x[1..1] == '/' and x[2..2] == "s"
			ret='S' if x[1..1] == 's'
			ret
		 }.join
		 
		# example is DdSsSsSsSsDdDDDDdDDSsSsDSsSsSsSsSsSsSsSsSsSsSsdSsDSsSsSsDSsdSsSsSsdSsDSsSsSsSsSsSsSsdSsDSsSsSsSsSsSsSsSsdSsDSsSsSsSsSsSsdSsDSsdSsDSsSsSsSsSsSsSsdddddDDDDDDddDDSDdsdDSDdsSDSsSSssSsdsddddDdddDDddd
		# each D represents a <div> and each d represents a </div>
		hash=Digest::MD5.hexdigest(binary)
		[{:probability=>100,:name=>"div structure",:string=>hash}]
	else
		[]
	end
end

end

