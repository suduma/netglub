##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "CushyCMS" do
author "Andrew Horton"
version "0.1"
description "Simple, closed-source CMS. Free and paid versions. Built with Ruby on Rails. Homepage: http://www.cushycms.com/"

examples %w| www.cushycms.com http://www.speldhurstderby.org.uk/ http://www.avantgardesalon.net/ www.zundoo.com http://www.zahirahbellydance.net http://www.nolimitsportfishing.com/ www.royalhousecatering.com dorenda.eu| 

# Set-Cookie: _cushy_session=BAh7CDoPc2Vzc2lvbl9pZCIlYTkwOTgxZGRmNTVmMjY3ZGRmYWJiNDhkNzM4OWY2NzQ6DXJlZmVycmVyIgAiCmZsYXNoSUM6J0FjdGlvbkNvbnRyb2xsZXI6OkZsYXNoOjpGbGFzaEhhc2h7AAY6CkB1c2VkewA%3D--e520ebeee34faf95fede4247ecaaf46fe80a84de; path=/; HttpOnly
#  <li id="poweredBy"><img alt="Powered by CushyCMS" src="/images/cushy_badge.gif?1274163122" /></li>

# this plugin wasn't made with enough samples... so it will probably miss some sites

matches [
{:name=>"poweredBy", 
:probability=>100,
:text=>'Content Management Powered by <a href="http://www.cushycms.com">CushyCMS</a>'},

{:name=>"poweredBy2",
:probability=>100,
:text=>'<li id="poweredBy"><img alt="Powered by CushyCMS" src="/images/cushy_badge.gif'},

{:name=>"poweredBy3",
:probability=>100,
:text=>'<span id="cushycms-footer">Powered by CushyCMS</span>'},

{:name=>"poweredBy4",
:probability=>100,
:regexp=>/<a href="http:\/\/www.cushycms.com\/[^>]+>(<font[^>]+>)?Powered by CushyCMS/}, #"

{:name=>"class=cushycms",
:probability=>100,
:regexp=>/<[^>]+class="cushycms"/}
]

def passive
  m=[]
  m << {:name=>"Cushy Cookie", :probability=>100 } if @meta["set-cookie"] =~ /_cushy_session=.*/
  m
end

end

