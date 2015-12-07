##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Index-Of" do
author "Andrew Horton"
version "0.1"
description "Index of"
examples %w| http://203.167.225.160 http://www.ibiblio.org/pub/ http://www.w3.org/Icons/ loop-aes.sourceforge.net http://www.gap-system.org/~history/Mathematicians/ http://www.lightrail.com/ http://download.opensuse.org/repositories/ |

# Apache2 http://www.ibiblio.org/pub/
#<a href="?C=N;O=D">Name</a>                     <a href="?C=M;O=A">Last modified</a>

# Apache2.2
# <th><a href="?C=N;O=D">Name</a></th><th><a href="?C=M;O=A">Last modified</a></th>

# Apache/2.2.3 (CentOS) Server at loop-aes.sourceforge.net
# <pre>Name                         Last modified      Size  <hr>

# Apache/2.0.52 (Unix) Server at www.gap-system.org
# <a href="?C=N;O=D">Name</a>                    <a href="?C=M;O=A">Last modified</a> 

# Apache/1.3.26 Server at www.czyborra.com
# <A HREF="?N=D">Name</A>                    <A HREF="?M=A">Last modified</A>

matches [
{:name=>"title", 
:probability=>100,
:text=>'<title>Index of /' },

{:name=>"name, last modified, catch-all",
:probability=>75,
:ghdb=>'name "last modified" size description' },

{:name=>"name, last modified",
:probability=>100,
:text=>'<a href="?C=N;O=D">Name</a></th><th><a href="?C=M;O=A">Last modified</a>' },

{:name=>"name, last modified2", 
:probability=>100,
:regexp=>/<a href="\?C=N;O=D">Name<\/a>[\s]*<a href="\?C=M;O=A">Last modified<\/a>/ },

{:name=>"name, last modified3", 
:probability=>100,
:regexp=>/<pre>Name[\s]+Last modified[\s]+Size[\s]+<hr>/ },

{:name=>"name, last modified4", 
:probability=>100,
:regexp=>/<A HREF="\?N=D">Name<\/A>[\s]+<A HREF="\?M=A">Last modified<\/A>/ },

{:name=>"minimal apache listing",
:probability=>100,
:regexp=>/<title>Index of \/[^<]*<\/title>[\s]*<\/head>[\s]*<body>[\s]*<h1>Index of / }

]

end

