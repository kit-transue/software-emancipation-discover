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

require 'wx'

class AccessFrame < Wx::Frame
  def initialize(factory)
    super(nil, :title => 'Discover Access', :size => [ 600, 400 ])
    @controller = factory.access_controller(self)
    
    splitter = Wx::SplitterWindow.new(self, -1)

    @result_text = Wx::TextCtrl.new(splitter, -1, '',
                                   Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE,
                                   Wx::TE_MULTILINE | Wx::TE_READONLY)

    @command_text = Wx::TextCtrl.new(splitter, -1, '',
                                   Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE,
                                   Wx::TE_MULTILINE)
    @command_text.evt_char {|event| @controller.on_evt_char(event)}

    splitter.set_minimum_pane_size(20)
    splitter.split_horizontally(@result_text, @command_text, 100)

  end

  def get_command
    @command_text.get_value
  end

  def set_command(cmd)
    @command_text.set_value(cmd)
  end

  def append_result(text)
    @result_text.append_text(text)
  end
end
