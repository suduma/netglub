##
# This file is part of WhatWeb and may be subject to
# redistribution and commercial restrictions. Please see the WhatWeb
# web site for more information on licensing and terms of use.
# http://www.morningstarsecurity.com/research/whatweb
##
Plugin.define "Plone" do
author "Andrew Horton"
version "0.1"
description "CMS http://plone.org"
examples %w| www.norden.org www.trolltech.com www.plone.net www.smeal.psu.edu|

matches [
{:name=>"meta generator tag",
:probability=>100,
:regexp=>/<meta name="generator" content="[^>]*http:\/\/plone.org" \/>/},

{:name=>"plone css",
:probability=>100,
:regexp=>/(@import url|text\/css)[^>]*portal_css\/.*plone.*css(\)|")/}, #"

{:name=>"plone javascript",
:probability=>100,
:regexp=>/src="[^"]*ploneScripts[0-9]+.js"/}, #"

{:name=>"div class=\"visualIcon contenttype-plone-site\"",
:probability=>100,
:text=>'<div class="visualIcon contenttype-plone-site">'},

{:name=>"div tag, visual-portal-wrapper",
:probability=>75,
:text=>'<div id="visual-portal-wrapper">'},
]

def passive
	m=[]
	#X-Caching-Rule-Id: plone-content-types
	#X-Cache-Rule: plone-content-types
	m << {:name=>"X-Caching-Rule-Id: plone-content-types", :probability=>100 } if @meta["x-caching-rule-id"] =~ /plone-content-types/i
	m << {:name=>"X-Cache-Rule: plone-content-types", :probability=>100 } if @meta["x-cache-rule"] =~ /plone-content-types/i
	m
end


end

