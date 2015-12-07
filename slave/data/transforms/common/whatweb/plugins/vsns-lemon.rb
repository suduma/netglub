##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "VSNSLemon" do
author "Andrew Horton"
version "0.1"
description "VSNS is a Very Simple News System written in PHP. VSNS Lemon vulnerabilities: http://evuln.com/vulns/106/summary.html"
examples %w|http://tachyondecay.net/blog/ www.mollysgusher.net/blog http://www.borregofam.com/blog http://papeh.net/blog/archives/Code/ |

# from version 4 at tachyondecay.net
#<img src="/images/vsns_lemon_80x15.png" alt="VSNS Lemon" title="Blog powered by VSNS Lemon" width="80" height="15">
#<a href="/scripts/vsns_lemon" title="Blog powered by VSNS Lemon"><img src="/images/vsns_lemon_80x15.png" alt="VSNS Lemon" width="80" height="15"></a>

# <div id="vsns-container">
# <div id="vsns-inner">
# <div class="news_container" id="vsns2527">


matches [
# http://johnny.ihackstuff.com/ghdb?function=detail&id=1840
{:name=>"GHDB: \"Powered by Vsns Lemon\" intitle:\"Vsns Lemon\"", 
:probability=>100,
:ghdb=>'"Powered by Vsns Lemon" intitle:"Vsns Lemon"'},

{:name=>"GHDB: test", 
:probability=>100,
:ghdb=>'<p>Powered by <abbr title="very simple news system">'},

{:name=>"powered by link",
:probability=>100,
:regexp=>/<p>Powered by <abbr title="very simple news system">VSNS<\/abbr> Lemon [0-9.a-z]* by <a href="http:\/\/tachyondecay.net\/">Tachyon<\/a>/},

{:name=>"link title = Blog powered by VSNS Lemon",
:probability=>100,
:regexp=>/<a[^>]*title="Blog powered by VSNS Lemon">/},

{:name=>"div id is vsns-wrapper, vsns-container or vsns-inner",
:probability=>50,
:regexp=>/<div id="(vsns-inner|vsns-container|vsns-inner)">/},

{:name=>"powered by VSNS image",
:probability=>100,
:regexp=>/<img[^>]*alt="VSNS Lemon"[^>]*title="Blog powered by VSNS Lemon"[^>]*>/}
]

def passive
	m=[]
	
	if @body =~ /<p>Powered by <abbr title="very simple news system">VSNS<\/abbr> Lemon [0-9.a-z]* by <a href="http:\/\/tachyondecay.net\/">Tachyon<\/a>/
		version=@body.scan(/<p>Powered by <abbr title="very simple news system">VSNS<\/abbr> (Lemon) ([0-9.a-z]*) by <a href="http:\/\/tachyondecay.net\/">Tachyon<\/a>/)[0][1]
		m << {:probability=>100,:name=>"powered by link",:version=>version} 
	end
	
	m
end

end

