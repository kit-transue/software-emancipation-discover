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

# XXX: violates Law of Demeter liberally

class BrowserController
  def initialize(model, view, queries, groups, editor, ast_viewer)
    @model = model
    @view = view
    @groups = groups
    @editor = editor
    @ast_viewer = ast_viewer

    @query_map = Hash.new
    queries.each do |category, sub_queries|
      sub_queries.each do |text, command|
        @query_map["#{category}-#{text}"] = command
      end
    end
  end

  def select(entities, members)
    changed = @model.set_selection(entities, members)
    if (changed) then
      clear_queries
    else
      run_queries
    end

    set_status(entities, members)
  end

  def set_status(entities, members)
    @view.set_status(if !members then
                       entities.size.to_s
                     elsif members.size == 1 then
                       entities.summary_text
                     else
                       members.size.to_s
                     end)
  end

  def query(wid_array)
    @model.set_queries(wid_array)
    run_queries
  end

  def run_queries
    queries = @model.get_queries
    return if queries.size == 0

    current = @model.current
    # shift if necessary
    query = queries.map {|elem| @query_map[elem]}
    @model.get_entities(1).query!(query, current)
    @view.refresh_lower
  end

  def clear_queries
    @view.clear_queries
    @model.clear_queries
  end

  def activate(entities)
    file, line = entities.source_pos
    @editor.open(file, line)
  end

  def ast
    @ast_viewer.display(@model.current)
  end

  ################
  #### Nav Buttons
  ################
  def set_upper(entity_set, members = nil)
    upper_entities = @model.get_entities(0)
    upper_entities.set_elems(entity_set, members) if entity_set
    clear_queries
    @view.refresh_upper
  end

  def roots
    clear_queries
    upper_entities = @model.get_entities(0)
    upper_entities.elems.roots!
    select(upper_entities, nil)
    @view.refresh_upper
  end

  def push
    clear_queries
    set_upper(@model.get_entities(1).elems, @model.get_selection(1))
  end

  def filter_set(entities, text)
    entities.elems.set_filter(text)
  end

  # XXX: Move to entity controller
  def back(entities)
    entities.back
  end

  def forward(entities)
    entities.forward
  end

  ################
  #### Groups
  ################
  def group_operation(operation, order = [0,1])
    operands = order.map do |index|
      @model.get_entities(index).tcl_name
    end

    @model.get_entities(1).assign_from!("#{operation} #{operands.join(' ')}")
    @view.refresh_lower
  end

  def group_union
    group_operation("set_union")
  end

  def group_intersect
    group_operation("set_intersect")
  end

  def group_subtract
    group_operation("set_subtract")
  end

  def group_subtract_ba
    group_operation("set_subtract", [1,0])
  end

  def new_group(title, entities)
    @groups.new_group(title, entities.elems, entities.selections)
  end

  def new_group_from_access(tcl_name)
    @groups.new_group_from_tcl(tcl_name)
  end

  def select_group(entities, group_index)
    entities.set_elems(@groups.at(group_index), nil)
  end
    
end
