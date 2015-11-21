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

require 'browser_history_model'
    
class MockEntityFactory
  def initialize(mock_array)
    @mock_array = mock_array
    @now_serving = 0
  end
  
  def entity_set
    @now_serving += 1
    @mock_array[@now_serving - 1]
  end
end

describe BrowserHistory do
  before(:each) do
    @mock_array = (0..10).collect { |x| mock("entity_#{x}")}
    @keep = 3
    @history = BrowserHistory.new(MockEntityFactory.new(@mock_array), @keep)
  end

  it "should start out reporting empty prev" do
    @history.prev.should eql(nil)
    @history.prev.should eql(nil)
  end

  it "should start out reporting empty next" do
    @history.next.should eql(nil)
    @history.next.should eql(nil)
  end

  it "supports push/prev" do
    @mock_array[0].should_receive(:copy!).with("dummy1")
    @history.push("dummy1")
    @mock_array[1].should_receive(:copy!).with("dummy2")
    @history.push("dummy2")

    @history.prev.should eql(@mock_array[1])
    @history.prev.should eql(@mock_array[0])
    @history.prev.should eql(nil)
    @history.prev.should eql(nil)
  end

  it "should keep only proscribed number of items" do
    max = 9
    max.should > @keep

    (0..max).each do |x|
      @mock_array[x].should_receive(:copy!)
      @mock_array[x - @keep].should_receive(:clear!) if (x - @keep >= 0)
      @history.push(x.to_s)
    end

    @history.next.should eql(nil)
    (0..@keep - 1).each do |x|
      @history.prev.should eql(@mock_array[max - x])
    end
    @history.prev.should eql(nil)
  end
end
