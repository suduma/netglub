##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "BlogSmithMedia" do
author "Andrew Horton"
version "0.1"
description "Pro bloggers - www.blogsmithmedia.com"

# identifying strings
# uses scripts,css,icons hosted at blogsmithmedia.com
# "<script .*\"http://www.blogsmithmedia.com" *

matches [
{:name=>"script loaded from www.blogsmithmedia.com",
:probability=>75,
:regexp=>/<script [^>]*\"http:\/\/www.blogsmithmedia.com/},
]


end




