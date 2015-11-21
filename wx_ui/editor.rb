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

require 'thread'

################################################################
#### Controller
################################################################

class EditorController
  def initialize(editor, entity_set)
    @editor = editor
    @scratch = entity_set
    @browser = nil
  end

  def parse_command(cmd)
    command, *args = cmd.split("\t")
    result_flag, action = command.split(' ')
    if (result_flag == '1') then
    else
      case action
        # when "pid_reply"
        # when "save_hook"
        # when "kill_hook"
        # when "activate"
        # when "cmput_reply"
        # when "editor_get"
        # when "popup"
      when "inst_def"
        inst_def(*args)
      when "query"
        send_to_browser(*args)
      when "open_def"
        open_def(*args)
      when "instances"
      when "right_click"
      else
        puts "unimplemented editor callback #{action}"
      end
    end
  end

  def callback_proc
    method(:parse_command)
  end

  def set_browser(browser)
    @browser = browser
  end

  def inst_def(file, offset, line, column)
    @scratch.assign_from!("get_instance_symbol [get_current_inst #{file} #{line},#{column}]")
    @editor.message(@scratch.query('sym_info', nil))
  end

  def open_def(file, offset, line, column)
    @scratch.assign_from!("get_instance_symbol [get_current_inst #{file} #{line},#{column}]")
    line = @scratch.query("apply line ", [0])
    file = @scratch.query("apply fname ", [0])
    @editor.open(file, line)
  end

  def send_to_browser(file, offset, line, column)
    if @browser then
      @scratch.assign_from!("get_instance_symbol [get_current_inst #{file} #{line},#{column}]")
      @browser.set_upper(@scratch)
    end
  end
end

################################################################
#### View
################################################################

class Editor
  def start_listener(callback)
  end

  def send(message)
    puts(message)
  end

  def open(file, line = nil)
    line ||= 0
    send("open\t#{file}\t#{line.to_s}")
  end

  def message(text)
    send("info\t#{text}")
  end
end

class EmacsEditor < Editor
  @@adapter = 'devxAdapter emacs /Applications/Emacs.app/Contents/MacOS/Emacs -l /Users/ktransue/PSETHOME/lib/dislite/dislite.el'

  def initialize
    @pipe = IO.popen(@@adapter, 'r+')
  end

  def start_listener(callback)
    Thread.new do
      while true do
        callback.call(@pipe.gets)
      end
    end
  end

  def send(message)
    @pipe.puts(message)
    @pipe.flush
  end
end

# e = Editor.new
# e.open('/Users/ktransue/cov-ast-print.txt', 5)
# sleep (200)
