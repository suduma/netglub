##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "NovellGroupwise" do
author "Andrew Horton"
version "0.1"
# identifying strings
# <!-- START NOVELL SERVICES -->
# <!-- START GROUPWISE WEBACCESS -->
# <!-- ========== GroupWise WebAccess Form ========== -->
# <b>GroupWise WebAccess:</b>
# &copy; Copyright 1993-2006 Novell, Inc. All rights reserved
# <title>Novell Web Services</title>


matches [
{:name=>"start novell services",
:probability=>100,
:text=>"<!-- START NOVELL SERVICES -->"},

{:name=>"start groupwise services",
:probability=>100,
:text=>"<!-- START GROUPWISE WEBACCESS -->"},

{:name=>"intitle:Novell Web Services",
:probability=>100,
:text=>"<title>Novell Web Services</title>"}
]


end

