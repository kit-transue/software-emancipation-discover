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

require 'browser_model'

describe BrowserEntity do
  before(:each) do
    @elems = mock('elems')
    @entities = BrowserEntity.new(@elems)
  end

  it 'should get file/line from model' do
    selection = 42
    line = 43
    file = 'foo.c'

    @elems.should_receive(:query).
      with('apply line ', [selection]).
      and_return(line)
    @elems.should_receive(:query).
      with('apply fname ', [selection]).
      and_return(file)

    @entities.source_pos(selection).should eql([file, line])
  end
end
    
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

describe BrowserModel do
  before(:each) do
    @mock_array = (0..10).collect { |x| mock("entity_#{x}")}
    @model = BrowserModel.new(MockEntityFactory.new(@mock_array))
  end

  it 'should not report unchanged selections' do
    entities_1 = stub('entities_1')
    entities_1.stub!(:selections=)
    entities_2 = stub('entities_2')
    entities_2.stub!(:selections=)

    @model.set_selection(entities_1, nil).should equal(entities_1)
    @model.set_selection(entities_1, nil).should be_nil
    @model.set_selection(entities_1, nil).should be_nil
    @model.set_selection(entities_2, nil).should equal(entities_2)
    @model.set_selection(entities_2, nil).should be_nil
    @model.set_selection(entities_1, nil).should equal(entities_1)
  end
end
