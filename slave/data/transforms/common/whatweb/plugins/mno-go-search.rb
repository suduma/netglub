##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "MnoGoSearch" do
author "Andrew Horton"
version "0.1"

description "mnoGoSearch is an opensource website search engine. Versions 3.1.19 to 3.2.15 are vulnerable, http://www.securityfocus.com/bid/9667. http://johnny.ihackstuff.com/ghdb/?function=detail&id=65. Homepage www.mnogosearch.org"

examples %w|http://www.uic.asso.fr/mnogosearch http://www.bio-systems.org/cgi-bin/search.cgi http://www.accessinn.com/cgi-bin/search.cgi http://www.zmanda.com/cgi-bin/search.cgi http://physhun.kfki.hu/cgi-bin/search-cncsun.cgi |

matches [
{:name=>"powered by", 
:probability=>100,
:ghdb=>'"Powered by mnoGoSearch – free web search engine software"'},


{:name=>"Page title starts with mnoGoSearch:/", 
:probability=>75,
:regexp=>/<title>mnoGoSearch:/},
]


end

