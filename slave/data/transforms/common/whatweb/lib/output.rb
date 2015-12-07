=begin
Copyright 2009, 2010 Andrew Horton

This file is part of WhatWeb.

WhatWeb is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
at your option) any later version.

WhatWeb is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with WhatWeb.  If not, see <http://www.gnu.org/licenses/>.
=end

class Output
	def initialize(f=STDOUT)
	# if no f, output to STDOUT, if f is a filename then open it, if f is a file use it
	
		@f = f if f.class == IO or f.class == File
		@f=File.open(f,"a") if f.class == String
		
	end
end

class OutputFull < Output
	def out(target, status, results) 
		@f.puts "Identifying: "+ target.to_s
		@f.puts "HTTP-Status: "+ status.to_s
		@f.puts results.pretty_inspect unless results.empty?	
		@f.puts
	end
end

class OutputBrief < Output
# don't use colours if not to STDOUT
	def out(target, status, results)
		brief_results=[]

# sort results so plugins that are less important at a glance are last
		last_plugins=%w| CSS MD5 Header-Hash Footer-Hash Div-Span-Structure|
		results=results.sort_by {|x,y| last_plugins.include?(x) ? 1 : 0 }


		results.each do |plugin_name,plugin_results|
			unless plugin_results.empty?
				# important info in brief mode is version, type and ?
				# what's the highest probability for the match?
				prob = plugin_results.map {|x| x[:probability] }.compact.sort.uniq.last					
				version = plugin_results.map {|x| x[:version] }.compact.sort.uniq.join(",")
				string = plugin_results.map {|x| x[:string] }.compact.sort.uniq.join(",")
				accounts = plugin_results.map {|x| [x[:account],x[:accounts] ] }.flatten.compact.sort.uniq.join(",")
			
				# be more DRY		
				if (@f == STDOUT and $use_colour=="auto") or ($use_colour=="always")
					 coloured_string = yellow(string)
					 coloured_string = cyan(string) if plugin_name == "HTTPServer"
 				 	 coloured_string = dark_green(string) if plugin_name == "Title"
 				 	 coloured_string = grey(string) if plugin_name == "MD5" 				 	 
 				 	 coloured_string = grey(string) if plugin_name == "Div-Span-Structure" 				 	 
 				 	 coloured_string = grey(string) if plugin_name == "Header-Hash"
 				 	 coloured_string = grey(string) if plugin_name == "Footer-Hash"
 				 	 coloured_string = grey(string) if plugin_name == "CSS"
 				 	  				 	  				 	 					 
					 coloured_plugin = white(plugin_name)
					 coloured_plugin = grey(plugin_name) if plugin_name == "MD5"
 					 coloured_plugin = grey(plugin_name) if plugin_name == "Div-Span-Structure"
  					 coloured_plugin = grey(plugin_name) if plugin_name == "Header-Hash"
  					 coloured_plugin = grey(plugin_name) if plugin_name == "Footer-Hash"  					 
  					 coloured_plugin = grey(plugin_name) if plugin_name == "CSS"
  					 					 
					 p = (prob!=100 ? grey(probability_to_words(prob))+ " " : "")  +
					   coloured_plugin + (!version.empty? ? "["+green(version)+"]" : "") +
					   (!string.empty? ? "[" + coloured_string+"]" : "") + (!accounts.empty? ? "["+ accounts+"]" : "" )
					 
					 brief_results << p
				else
					brief_results << (prob!=100 ? probability_to_words(prob)+ " " : "")  +
					   plugin_name + (!version.empty? ? "[" + version +"]" : "") +
					   (!string.empty? ? "[" + string+"]" : "") + (!accounts.empty? ? " ["+ accounts+"]" : "" )
				end
			end
		end
		
		if (@f == STDOUT and $use_colour=="auto") or ($use_colour=="always")
			@f.puts blue(target) + " [#{status}] " + brief_results.join(", ")
		else
			@f.puts target.to_s + " [#{status}] " + brief_results.join(", ")
		end		
		
	end
end


class OutputXML < Output
	def out(target, status, results) 
		
		@f.puts "<target>"
		@f.puts "\t<uri>#{target}</uri>"
		@f.puts "\t<http-status>#{status}</http-status>"
		
		results.each do |plugin_name,plugin_results|		
			@f.puts "\t<plugin>"
			@f.puts "\t\t<name>#{plugin_name}</name>"
			
			unless plugin_results.empty?
				# important info in brief mode is version, type and ?
				# what's the highest probability for the match?
				prob = plugin_results.map {|x| x[:probability] }.compact.sort.uniq.last
				version = plugin_results.map {|x| x[:version] }.flatten.compact.sort.uniq
				string = plugin_results.map {|x| x[:string] }.flatten.compact.sort.uniq
				accounts = plugin_results.map {|x| [x[:account],x[:accounts] ] }.flatten.compact.sort.uniq

				@f.puts "\t\t<certainty>#{prob}</certainty>" unless certainty= 100				
				version.map  {|x| @f.puts "\t\t<version>#{x}</version>"  }			
				string.map   {|x| @f.puts "\t\t<string>#{x}</string>" }				
				accounts.map {|x| @f.puts "\t\t<accounts>#{x}</accounts>" }
			end
					
			@f.puts "\t</plugin>"
		end
		@f.puts "</target>"
	end
end


