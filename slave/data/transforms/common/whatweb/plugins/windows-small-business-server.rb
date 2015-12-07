##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "WindowsSBS" do
author "Andrew Horton"
version "0.1"
description "Microsoft Small Business Server Homepage:www.microsoft.com/sbs/en/us/default.aspx"
examples %w| 210.54.213.143 cgamining.com phillipfparsons.com www.bonesucks.com  |

# this needs more examples

# <title>Welcome to Windows Small Business Server 2003</title>
# these are all 2003

matches [
{:name=>"title",
:probability=>100,
:version=>2003,
:regexp=>/<title>Welcome to Windows Small Business Server 2003<\/title>/ },

{:name=>"Remote Web Workplace link",
:probability=>100,
:regexp=>/<TD id="Remote_Link" class="linkHeader"><A HREF="\/Remote">Remote Web Workplace<\/A><\/TD>/ },

{:name=>"sbslogo.gif",
:probability=>75,
:regexp=>/<IMG alt="" src="images\/sbslogo.gif" border="0" width="200" height="55">/ }

]


end

