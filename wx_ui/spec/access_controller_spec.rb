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

require 'access_controller'
    
describe AccessController do
  before(:each) do
    @history = mock('history')
    @server = mock('server')
    @view = mock('view')
    @controller = AccessController.new(@server, @history, @view)
  end

  it 'should skip normal keystrokes' do
    "Normal input".each_byte do |ch|
      event = mock('event')
      event.should_receive(:get_key_code).and_return(ch)
      event.should_receive(:skip)
      @controller.on_evt_char(event)
    end
  end

  it 'should set history on up-arrow' do
    text = 'previous'
    @history.should_receive(:prev).and_return(text)
    @view.should_receive(:set_command).with(text)

    event = mock('event')
    event.should_receive(:get_key_code).and_return(Wx::K_UP)
    
    @controller.on_evt_char(event)
  end

  it 'should set history on down-arrow' do
    text = 'next'
    @history.should_receive(:next).and_return(text)
    @view.should_receive(:set_command).with(text)

    event = mock('event')
    event.should_receive(:get_key_code).and_return(Wx::K_DOWN)
    
    @controller.on_evt_char(event)
  end

  it 'should run command on CR' do
    event = mock('event')
    event.should_receive(:get_key_code).and_return(Wx::K_RETURN)
    
    @controller.should_receive(:run_command)
    @controller.on_evt_char(event)
  end

  it 'should save history and print command & results when running commands' do
    cmd_text = "this is a command"
    result = ["multi-line", "result text"]

    @server.should_receive(:send_command).with(cmd_text).ordered
    @server.should_receive(:get_result).ordered.and_return(result)

    @view.should_receive(:get_command).ordered.and_return(cmd_text)
    @view.should_receive(:set_command).with("").ordered
    @view.should_receive(:append_result).with(cmd_text + "\n").ordered
    @view.should_receive(:append_result).with(result.join("\n") + "\n").ordered

    @history.should_receive(:push).with(cmd_text)

    @controller.run_command
  end
end
