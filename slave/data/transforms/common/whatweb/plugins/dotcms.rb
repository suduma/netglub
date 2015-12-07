##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "DotCMS" do
author "Andrew Horton"
version "0.1"
description "DotCMS is an opensource CMS written in Java. Has enterprise support. Homepage: http://www.dotcms.org/"

examples %w|
education.asu.edu
www.astate.edu/
www.bvu.edu/
www.columbustech.edu/
www.edinboro.edu/
www.gettysburg.edu/
www.greenville.edu/
www.keystone.edu/
www.msoe.edu/
www.mtaloy.edu/
www.northwestcollege.edu/
www.oberlin.edu/
oxford.emory.edu/
www.pittstate.edu/
www.sjfc.edu/
www.taylor.edu/
www.uakron.edu/
www.uthouston.edu/
|

# <img src="/resize_image?path=/dotAsset/
# <img src="/dotAsset/
# <link href="/dotAsset/


matches [
{:name=>"img dotAsset/", 
:probability=>100,
:regexp=>/<img[^>]+src="[^"]*\/dotAsset\//}, #"

{:name=>"link dotAsset/", 
:probability=>100,
:regexp=>/<link[^>]+href="[^"]*\/dotAsset\//}, #"

{:name=>"local link to index.dot",
:probability=>100,
:regexp=>/<a[^>]+href="[^h][^"]*index\.dot/} #"
]

end


