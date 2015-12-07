##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "SnomPhone" do
author "Andrew Horton"
version "0.1"
description "voip phone homepage:www.snom.com"
examples %w| http://121.73.14.125/ http://63.117.239.171/|


# &copy; 2000-2008 <a href="http://snom.com">snom AG</a><br>
# <tr><td class="flyoutLink" colspan="2"><b><a href=http://wiki.snom.com/wiki/index.php/snom300>Manual</a>
# You can enter a simple telephone number (e.g. 0114930398330) or URI like info@snom.com.
# <td class="headerText" width="705">Welcome to Your Phone!</td>
# <tr><td class="flyoutLink" colspan="2"><b><a href=http://www.snom.com/wiki/index.php/snom360>Manual</a>


matches [
{:name=>"copyright snom.com", 
:probability=>100,
:regexp=>/&copy; 2000-20[0-9]+ <a href="http:\/\/snom.com">snom AG<\/a><br>/ },

{:name=>"link to manual1", 
:probability=>100,
:regexp=>/<tr><td class="flyoutLink" colspan="2"><b><a href=http:\/\/(wiki|www).snom.com/ },

{:name=>"You can enter a simple telephone number (e.g. 0114930398330)...", 
:probability=>100,
:regexp=>/You can enter a simple telephone number \(e.g. 0114930398330\) or URI like info@snom.com./ },

{:name=>"Welcome to Your Phone!", 
:probability=>75,
:regexp=>/<td[^>]+>Welcome to Your Phone!<\/td>/ },


]



end



