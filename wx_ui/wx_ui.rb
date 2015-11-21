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

require 'wx'

require 'access_controller'
require 'access_history_model'
require 'access_view'
require 'browser_controller'
require 'browser_model'
require 'browser_view'
require 'dish_client'
require 'editor'
require 'group_model'
require 'ast_viewer'



################################################################
#### Factories/IOC
################################################################

class ProductionFactory
 def initialize(service = nil)
   @server = Server.new(service)
   @last_entity_id = -1
   @editor = build_editor()
   @edit_controller = EditorController.new(@editor, entity_set)
   @editor.start_listener(@edit_controller.callback_proc)
   @ast_viewer = AstViewer.new
 end

 def build_editor
   if ENV['PSETHOME'] && FileTest::exist?("#{ENV['PSETHOME']}/bin/devxAdapter") then
     EmacsEditor.new
   else
     Editor.new
   end
 end

 def named_entity_set(name)
   EntitySet.new(@server, name)
 end

 def entity_set
   @last_entity_id += 1
   es = named_entity_set("set_#{@last_entity_id}")
   es.roots!
   es
 end

 def access_controller(view)
   AccessController.new(@server, AccessHistory.new, view)
 end

 def editor_controller
   @edit_controller
 end

 def browser_model
   BrowserModel.new(self)
 end

 def browser_controller(model, view, queries, groups)
   controller = BrowserController.new(model, view, queries, groups,
                                      @editor,
                                      @ast_viewer)
   @edit_controller.set_browser(controller)
   controller
 end

 def groups
   Groups.new(self)
 end
end

################################################################
#### Do something
################################################################

Wx::App.run do 
 queries = {
   'where'   => [
                 ['defined',         'where defined'],
                 ['referenced',      'where referenced'],
                 ['used',            'where used'],
                 ['included',        'where included'],
                 ['declared',        'where declared'],
                ],
   'uses'    => [
                 ['includes',        'get_include_files'],
                 ['functions',       'uses -functions'],
                 ['classes',         'uses -classes'],
                 ['variables',       'uses -variables'],
                 ['locals',          'uses -locals'],
                 ['constants',       'uses -constants'],
                 ['strings',         'uses -strings'],
                 ['macros',          'uses -macros'],
                 ['typedefs',        'uses -typedefs'],
                 ['enums',           'uses -enums'],
                ],
   'defines' => [
                 ['all',             'defines -all'],
                 ['files',           'defines -files'],
                 ['functions',       'defines -functions'],
                 ['classes',         'defines -classes'],
                 ['variables',       'defines -variables'],
                 ['locals',          'defines -locals'],
                 ['constants',       'defines -constants'],
                 ['strings',         'defines -strings'],
                 ['macros',          'defines -macro'],
                 ['typedefs',        'defines -typedefs'],
                 ['enums',           'defines -enums'],
                ],
   'show' => [
                 ['instances',       'instances'],
                 ['super-class',     'get_super_classes'],
                 ['sub-class',       'get_sub_classes'],
                 ['super-method',    'get_method super'],
                 ['sub-method',      'get_method sub'],
                 ['overrides',       'method_overrides'],
                ],

 }

 self.app_name = 'Discover'
 factory = ProductionFactory.new(ARGV[0])
 groups = factory.groups
 frame = BrowserFrame.new(factory, "Discover", queries, groups)
 frame.show
# frame.newAccessFrame # XXX
end
