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

class BrowserEntity
  attr :selections, true
  attr :elems, true

  def tcl_name
    @elems.tcl_name(@selections)
  end
    
  def initialize(elems)
    @elems = elems
    @selections = nil
  end

  def size
    @elems.size
  end

  def source_pos
    line = query("apply line ")
    file = query("apply fname ")
    [file, line]
  end

  def summary_text
    @elems.query('sym_info', @selections)
  end

  def set_elems(data, selections)
    @elems.copy!(data, selections) if data
    @selections = nil
  end

  def assign_from!(query)
    @elems.assign_from!(query)
    @selections = nil
  end

  def query(command)
    @elems.query(command, @selections)
  end

  def query!(command, entities)
    @elems.query!(command, entities.elems, entities.selections)
  end
end

class BrowserEntityWithHistory < BrowserEntity
  def initialize(factory)
    super(factory.entity_set)
    @history = BrowserHistory.new(factory)
  end

  def assign_from!(query)
    push_to_history
    super(query)
  end
    
  def query!(query, entities)
    push_to_history
    super(query, entities)
  end

  def push_to_history
    @history.push(@elems)
  end

  def set_elems(entities, members, keep_history = true)
    @history.push(@elems) if keep_history
    super(entities, members)
  end

  def back
    @history.push(@elems) if @history.last?
    set_elems(@history.prev, nil, false)
  end

  def forward
    set_elems(@history.next, nil, false)
  end
end

class BrowserModel
  def initialize(factory)
    @entities = [BrowserEntityWithHistory.new(factory),
                 BrowserEntityWithHistory.new(factory)]
    @last_selected = nil

    clear_queries
  end

  def current
    @last_selected
  end

  def get_entities(index)
    @entities[index]
  end

  def get_selection(index)
    @entities[index].selections
  end

  def set_selection(entities, elements)  # returns true if changed
    entities.selections = elements
    @last_selected = entities if @last_selected != entities
  end

  # XXX: these look like attrs to me!
  def clear_queries
    @active_queries = []
  end

  def set_queries(wid_array)
    @active_queries = wid_array
  end

  def get_queries
    @active_queries
  end
end
