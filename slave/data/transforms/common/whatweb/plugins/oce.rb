##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Oce" do
author "Andrew Horton"
version "0.1"
description "Oce Print Exec Workgroup is easy-to-use, web-based print management software for job submission of sets of technical drawings to a single large format printer. Homepage: global.oce.com/products/print-exec-workgroup/default.aspx"
examples %w| 222.155.44.88 |

# http server 'PEWG/1.2'

# <title>Print Exec Workgroup</title>
# /servlet/owslhtml/owslicons/header_pewg.jpg

matches [
{:name=>"title", 
:probability=>100,
:regexp=>/<title>Print Exec Workgroup<\/title>/i },

{:name=>"header jpg", 
:probability=>100,
:regexp=>/\/servlet\/owslhtml\/owslicons\/header_pewg.jpg/ }

]


end

