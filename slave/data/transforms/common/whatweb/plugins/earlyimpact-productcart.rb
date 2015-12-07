##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "EarlyImpact-ProductCart" do
author "Andrew Horton"
version "0.1"
description "EarlyImpact ProductCart is an ASP commercial ecommerce system from www.earlyimpact.com. Version < 2.53 is vulnerable http://www.securityfocus.com/bid/9669  Googledork http://johnny.ihackstuff.com/ghdb/?function=detail&id=64"
examples %w|www.maunakeagalleries.com www.flys.com www.topcoins.com|

matches [
{:name=>"inurl:custva.asp", 
:probability=>75,
:ghdb=>'inurl:custva.asp'},

{:name=>"viewCat_h.asp link", 
:probability=>100,
:text=>"viewCat_h.asp"}
]


end

