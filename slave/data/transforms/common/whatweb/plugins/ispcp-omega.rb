##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "ispCP-Omega" do
author "Andrew Horton"
version "0.1"
description "PHP opensource, virtual hosting system homepage: http://www.isp-control.net/"
examples %w| 202.6.117.217 |

# <title>ispCP Omega a Virtual Hosting Control System</title>
# Powered by <a class="login" href="http://www.isp-control.net" target="_blank">ispCP Omega</a>
# themes/omega_original/images/login/login_lock.jpg


matches [
{:name=>"title", 
:probability=>100,
:text=>'<title>ispCP Omega a Virtual Hosting Control System</title>' },

{:name=>"powered by", 
:probability=>100,
:text=>'Powered by <a class="login" href="http://www.isp-control.net" target="_blank">ispCP Omega' },

{:name=>"path themes/omega_original/images",
:probability=>100,
:text=>'themes/omega_original/images/login/login_lock.jpg' }
]

end

