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

require 'browser_controller'

describe BrowserController do
  before(:each) do
    @model = mock('model')
    @view = mock('view')
    @groups = mock('groups')
    @editor = mock('editor')
    @queries = {
      'top1' => [
                 ['sub1-1', 'text 1-1'],
                 ['sub1-2', 'text 1-2'],
                ],
      'top2' => [
                 ['sub2-1', 'text 2-1'],
                 ['sub2-2', 'text 2-2'],
                ],
    }
    @controller = 
      BrowserController.new(@model, @view, @queries, @groups, @editor)
  end

  it 'should be constructable' do
  end

  it 'should navigate to file/line on activate' do
    selection = 42
    line = 43
    file = 'foo.c'
    entities = mock('entities')
    entities.should_receive(:source_pos).with(selection).
      and_return([file, line])

    @editor.should_receive(:open).with(file, line)

    @controller.activate(entities, selection)
  end

  ################
  # set_status
  ################
  it 'should report set size as status with no selections' do
    magic_size = 33
    entities = mock('entities')
    entities.should_receive(:size).and_return(magic_size)

    @view.should_receive(:set_status).with(magic_size.to_s)

    @controller.set_status(entities, nil)
  end

  it 'should report selection size as status with multiple selections' do
    magic_size = 21
    entities = mock('entities')
    members = mock('members')
    members.should_receive(:size).at_least(:once).and_return(magic_size)

    @view.should_receive(:set_status).with(magic_size.to_s)

    @controller.set_status(entities, members)
  end

  it 'should report symbol information with one selection' do
    symbol_index = 1414
    symbol_text = 'requested symbol info'

    entities = mock('entities')
    entities.should_receive(:summary_text).
      with([symbol_index]).
      and_return(symbol_text)

    @view.should_receive(:set_status).with(symbol_text)

    @controller.set_status(entities, [symbol_index])
  end

  ################
  # select
  ################
  it 'should clear queries if selection is from a new pane' do
    entities = mock('entities')
    selection = [9, 10, 11]

    @model.should_receive(:set_selection).
      with(entities, selection).
      and_return(true)
    @controller.should_receive(:set_status).with(entities, selection)
    @controller.should_receive(:clear_queries)
    @controller.should_not_receive(:run_queries)

    @controller.select(entities, selection)
  end

  it 'should run queries if selection is from existing panel' do
    entities = mock('entities')
    selection = [9, 10, 11]

    @model.should_receive(:set_selection).
      with(entities, selection).
      and_return(false)
    @controller.should_receive(:set_status).with(entities, selection)
    @controller.should_receive(:run_queries)
    @controller.should_not_receive(:clear_queries)

    @controller.select(entities, selection)
  end

  ################
  # query
  ################
  it 'should do nothing with empty queries' do
    @model.should_receive(:get_queries).and_return([])

    @controller.run_queries
  end

  it 'should run queries _elegantly_?' do
    elems_0 = mock('elems_0')
    elems_1 = mock('elems_1')

    @model.stub!(:current).and_return(elems_0)
    @model.stub!(:get_entities).with(1).and_return(elems_1)

    @model.should_receive(:get_queries).
      and_return(['top1-sub1-1', 'top2-sub2-1'])

    elems_1.should_receive(:query!).with(['text 1-1', 'text 2-1'], elems_0)

    @view.should_receive(:refresh_lower)
    @controller.run_queries
  end

  it 'should do group operations' do
    elems_0 = mock('elems_0')
    elems_1 = mock('elems_1')

    @model.stub!(:get_entities).with(0).and_return(elems_0)
    @model.stub!(:get_entities).with(1).and_return(elems_1)

    elems_0.should_receive(:tcl_name).and_return("DI_UP")
    elems_1.should_receive(:tcl_name).and_return("DI_DOWN{1,2}")

    elems_1.should_receive(:assign_from!).with("foo_op DI_UP DI_DOWN{1,2}")

    @view.should_receive(:refresh_lower)

    @controller.group_operation("foo_op")
  end

  it 'should do group operation with alternate sequence' do
    elems_0 = mock('elems_0')
    elems_1 = mock('elems_1')

    @model.stub!(:get_entities).with(0).and_return(elems_0)
    @model.stub!(:get_entities).with(1).and_return(elems_1)

    elems_0.should_receive(:tcl_name).and_return("DI_UP")
    elems_1.should_receive(:tcl_name).and_return("DI_DOWN")

    elems_1.should_receive(:assign_from!).with("foo_op DI_DOWN DI_UP")

    @view.should_receive(:refresh_lower)

    @controller.group_operation('foo_op', [1,0])
  end

end
