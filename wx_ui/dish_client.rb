#!/usr/bin/env ruby -rubygems
##########################################################################
# Copyright (c) 2015, Synopsys, Inc.                                     #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions are #
# met:                                                                   #
#                                                                        #
# 1. Redistributions of source code must retain the above copyright      #
# notice, this list of conditions and the following disclaimer.          #
#                                                                        #
# 2. Redistributions in binary form must reproduce the above copyright   #
# notice, this list of conditions and the following disclaimer in the    #
# documentation and/or other materials provided with the distribution.   #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  #
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   #
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, #
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       #
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  #
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  #
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    #
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  #
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   #
##########################################################################


################################################################
#### Model
################################################################

class Server
  @@dish = 'dish2'    # XXX: concern about path?

  def initialize(service = nil)
    service ||= Server.get_services.chomp!

    @pipe = IO.popen("#{@@dish} #{service} -dislite", 'r+')
    # XXX: send equivalent of dis_server.tcl to set printformat, install
    # functions that will chunk set transfers, etc.
    send_command('source ~/PSETHOME/lib/dislite.dis; puts "0\n% a"')
    @prompt_changed = true
    get_result
  end

  def self.get_services
    `#{@@dish}`
  end

  def send_command(command)
    @pipe.puts(command)
    @pipe.flush
  end

  def get_result
    ret = Array.new
    (1..@pipe.gets.to_i).each do |line|
      ret.push(@pipe.gets.chomp)
    end
    if (!@prompt_changed) then
      throw :error_receiving if ret.pop != '% '
    end
    ret
  end
end

class EntitySet
  def initialize(server, name)
    @server = server
    @name = name
    @filtered_name = "#{name}_filter"
    @cache = Array.new
    @filtered_cache = @cache
  end

  def tcl_name(members)
    member_string = members ? "{#{members.map {|x| x+1}.join(',')}}" : ''
    "$#{@filtered_name}#{member_string}"
  end

  def query(expr, members)
    @server.send_command("#{expr} #{tcl_name(members)}")
    @server.get_result.join("\n")
  end

  def query!(expr, operand, members = nil)
    mems = operand.tcl_name(members)
    if expr.kind_of?(Array) then
      todo = "set_unique" + expr.collect {|x| " [#{x} #{mems}]"}.join
    else
      todo = "#{expr} #{mems}"
    end
    assign_from!(todo)
  end

  def copy!(operand, members = nil)
    assign_from!('set_copy ' + operand.tcl_name(members))
  end
    
  def assign_from!(expr)
    @server.send_command("set #{@name} [#{expr}]")
    @cache = @server.get_result
    clear_filter
  end

  def clear!
    @server.send_command("set #{@name} 0")
    @server.get_result
    @server.send_command("set #{@filtered_name} 0")
    @server.get_result
    @cache = nil
    clear_filter
  end

  def roots!
    assign_from!('roots')
  end

  def size
    @filtered_cache.size
  end

  def get_text(index)
    @filtered_cache[index] || ''
  end

  def clear_filter
    @server.send_command("set #{@filtered_name} $#{@name}")
    @filtered_cache = @server.get_result
  end

  def set_filter(text) 
    if text == nil || text == '' then
      clear_filter
    else
      @server.send_command("set #{@filtered_name} [filter match(\"*#{text}*\") $#{@name}]")
      @filtered_cache = @server.get_result
    end
  end
end


#def q(lhs, op, rhs, mems = nil)
#  puts "-- #{op} --"
#  puts lhs.query(op, rhs, mems)
#end

#s = Server.new
#es = EntitySet.new(s, "top")
#es1 = EntitySet.new(s, "bottom")
#puts es.roots
#es.query('defines -functions', es)
#q(es1, ["uses -locals", "uses -functions"], es, (1..10).to_a)
#q(es, "where defined", es1)
#q(es, "where declared", es1)


# set management:
# dis_get_next_ind rotates round-robin through non-empty sets.  Empty
# sets are immediately reused

# $result(n) is kept locally. $result(tags_n), $result(sel_n), too.

# how does display_set work?
# - look at ui.tcl and util.tcl

# what does dis_server.tcl do?  How does it define the set output?
