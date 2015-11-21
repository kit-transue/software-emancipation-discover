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

require 'uwid'

class EntityListCtrl < Wx::ListCtrl
  def initialize (parent, entities, controller)
    super(parent, -1, Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE, Wx::LC_VIRTUAL)

    @entities = entities
    @controller = controller
    @select_count = 0

    # XXX: change column header to ListItem data.
    # XXX: add bitmaps
    insert_column(0, 'name', Wx::LIST_FORMAT_LEFT, 0);

    # XXX: we really want list_items_changed -> query_selection....
    evt_list_item_selected(self) {|event| on_item_selected}
    evt_list_item_deselected(self) {|event| on_item_deselected}
    evt_list_item_activated(self) {|event| on_item_activated}
    evt_set_focus {|event| on_focus}
    update_view
  end

  def get_selection
    # surely there's a better way of doing this!
    selections = Array.new
    
    item = get_next_item(-1, Wx::LIST_NEXT_ALL, Wx::LIST_STATE_SELECTED)
    while item != -1 do
      selections.push(item)
      item = get_next_item(item, Wx::LIST_NEXT_ALL, Wx::LIST_STATE_SELECTED)
    end
    selections.empty? ? nil : selections
  end
    
  def query_if_selection_complete
    # could make this faster w/ more overrides of ListCtrl
    if @select_count == get_selected_item_count then
      @controller.select(@entities, get_selection)
    end
  end

  def on_focus
    @controller.select(@entities, get_selection)
  end

  def on_item_selected
    @select_count += 1
    query_if_selection_complete
  end

  def on_item_deselected
    @select_count -= 1
    query_if_selection_complete
  end

  def on_item_activated
    @controller.activate(@entities)
  end

  def on_get_item_text(index, column)
    @entities.elems.get_text(index)
  end

  def update_view
    ensure_visible(0)
    delete_all_items
    set_item_count(@entities.size)
    @select_count = 0
  end
end

class EntityPanel < Wx::BoxSizer
  def initialize(parent, entities, controller, groups)
    super(Wx::VERTICAL)

    @groups = groups

    nav_bar = Wx::BoxSizer.new(Wx::HORIZONTAL)
    add(nav_bar)

    button = Wx::Button.new(parent, -1, '<')
    nav_bar.add(button, 1, Wx::EXPAND)
    parent.evt_button(button) do |event|
      controller.back(entities)
      @list.update_view
    end

    button = Wx::Button.new(parent, -1, '>')
    nav_bar.add(button, 1, Wx::EXPAND)
    parent.evt_button(button) do |event|
      controller.forward(entities)
      @list.update_view
    end

    @group_choice = Wx::Choice.new(parent, -1)
    nav_bar.add(@group_choice)
    parent.evt_choice(@group_choice) do |event|
      controller.select_group(entities, event.get_index)
      @list.update_view
    end

    button = Wx::Button.new(parent, -1, 'new')
    nav_bar.add(button)
    parent.evt_button(button) do |event|
      parent.new_group_popup(parent, controller, entities)
    end

    ctrl = Wx::TextCtrl.new(parent, -1, "", Wx::DEFAULT_POSITION)
    nav_bar.add(ctrl, 1, Wx::EXPAND)
    parent.evt_text(ctrl) do |event|
      controller.filter_set(entities, event.get_string())
      @list.update_view
    end

    @list = EntityListCtrl.new(parent, entities, controller)
    add(@list, 1, Wx::EXPAND)
  end

  def update_view
    @list.update_view
  end

  def refresh_groups
    @group_choice.clear
    @groups.list.each {|name| @group_choice.append(name) }
  end
end

class QueryPanel < Wx::BoxSizer
  def initialize(parent, controller, queries)
    super(Wx::VERTICAL)
    @query_buttons = Hash.new

    # don't want to use a grid since we're not going to fill evenly
    queries.each do |category, sub_queries|
      label = Wx::StaticText.new(parent, -1, category)
      add(label, 1, Wx::ALIGN_LEFT)
      sub_queries.each do |title_action_pair|
        text = title_action_pair[0]
        button = Wx::ToggleButton.new(parent, -1, text)
        @query_buttons[button] = "#{category}-#{text}"
        add(button, 1, Wx::EXPAND)
        parent.evt_togglebutton(button) do |event|
#          if !event.cmd_down then
#            clear-all-but-button
#          end
          controller.query(get_queries)
        end
      end
      label = Wx::StaticText.new(parent, -1, "")
      add(label, 1)
    end
  end

  def get_queries
    @query_buttons.collect{|button, query_id|
      button.get_value ? query_id : nil}.compact
  end

  def clear
    @query_buttons.each do |button, query_id|
      button.set_value(false)
    end
  end
end

class GroupPanel < Wx::BoxSizer
  def initialize(parent, controller, model)
    super(Wx::HORIZONTAL)
    @model = model

    button = Wx::Button.new(parent, -1, 'union')
    add(button)
    parent.evt_button(button) { |event| controller.group_union }

    button = Wx::Button.new(parent, -1, 'intersect')
    add(button)
    parent.evt_button(button) { |event| controller.group_intersect }

    button = Wx::Button.new(parent, -1, '^ - v')
    add(button)
    parent.evt_button(button) { |event| controller.group_subtract }

    button = Wx::Button.new(parent, -1, 'v - ^')
    add(button)
    parent.evt_button(button) { |event| controller.group_subtract_ba }
  end
end

class NavPanel < Wx::BoxSizer
  def initialize(parent, controller)
    super(Wx::HORIZONTAL)

    button = Wx::Button.new(parent, -1, 'roots')
    add(button, 1, Wx::EXPAND)
    parent.evt_button(button) {|event| controller.roots }

    button = Wx::Button.new(parent, -1, 'push')
    add(button, 1, Wx::EXPAND)
    parent.evt_button(button) {|event| controller.push }

    button = Wx::Button.new(parent, -1, 'AST')
    add(button, 1, Wx::EXPAND)
    parent.evt_button(button) {|event| controller.ast }
  end
end

class BrowserFrame < Wx::Frame
  def initialize(factory, title, queries, groups)
    super(nil, :title => title, :size => [ 550, 300 ])
    @groups = groups
    @factory = factory
    @model = factory.browser_model
    @controller = factory.browser_controller(@model, self, queries, groups)

    evt_timer(UWID[:timer_poll]) { Thread.pass }
    timer = Wx::Timer.new(self, UWID[:timer_poll])
    timer.start(25)


    access_menu = Wx::Menu.new
    access_menu.append(UWID[:access_frame], 'Access Window')
    access_menu.append(UWID[:access_group], 'Group from Access variable...')

    menu_bar = Wx::MenuBar.new
    menu_bar.append(access_menu, "&Access")
    set_menu_bar(menu_bar)
    evt_menu(UWID[:access_frame]) {|event| newAccessFrame }
    evt_menu(UWID[:access_group]) {|event| newAccessGroup(self, @controller) }
    evt_menu Wx::ID_EXIT, :on_quit
    evt_menu Wx::ID_ABOUT, :on_about

    left_sizer = Wx::BoxSizer.new(Wx::VERTICAL)  # XXX: use splitter instead
    left_sizer.add(NavPanel.new(self, @controller), 0, Wx::TOP, 6)
    left_sizer.add(GroupPanel.new(self, @controller, groups), 0, Wx::TOP, 6)
    entities = @model.get_entities(0)
    @list1 = EntityPanel.new(self, entities, @controller, groups)
    left_sizer.add(@list1, 1, Wx::EXPAND | Wx::TOP, 6)
    entities = @model.get_entities(1)
    @list2 = EntityPanel.new(self, entities, @controller, groups)
    left_sizer.add(@list2, 1, Wx::EXPAND | Wx::TOP, 6)
    @status = Wx::TextCtrl.new(self, -1, "",
                               Wx::DEFAULT_POSITION, Wx::DEFAULT_SIZE,
                               Wx::TE_READONLY)
    left_sizer.add(@status, 0, Wx::EXPAND | Wx::TOP, 6)

    @query_panel = QueryPanel.new(self, @controller, queries)
    parent_sizer = Wx::BoxSizer.new(Wx::HORIZONTAL)
    parent_sizer.add(left_sizer, 1, Wx::EXPAND)
    parent_sizer.add(@query_panel, 0)
    set_sizer(parent_sizer)
  end

  def refresh_upper
    @list1.update_view
  end

  def refresh_lower
    @list2.update_view
  end

  def refresh_groups
    [@list1, @list2].each do |list|
      list.refresh_groups
    end
  end
  
  def set_status(text)
    # XXX style: status doesn't exist in model; hard to pull.  Is push OK?
    @status.set_value(text)
  end

  def new_group_popup(parent, controller, entities)
    dlg = Wx::TextEntryDialog.new(parent, "Group name:", "New Group", "groupXX")
    if dlg.show_modal() == Wx::ID_OK
      controller.new_group(dlg.get_value().to_s, entities)
    end     
    dlg.destroy()
    refresh_groups
  end

  def newAccessFrame
    AccessFrame.new(@factory).show
  end

  def newAccessGroup(parent, controller)
    dlg = Wx::TextEntryDialog.new(parent, "Access variable name:", "New Group From Access Variable", "x")
    if dlg.show_modal() == Wx::ID_OK
      controller.new_group_from_access(dlg.get_value().to_s)
    end     
    dlg.destroy()
    refresh_groups
  end

  def clear_queries
    @query_panel.clear
  end

  def on_quit
    close()
  end

  def on_about
    about_dlg = Wx::MessageDialog.new( self,
                                       'Copyright 2008, Coverity, Inc.',
                                       'About Discover',
                                       Wx::OK|Wx::ICON_INFORMATION )
    about_dlg.show_modal
  end
end
