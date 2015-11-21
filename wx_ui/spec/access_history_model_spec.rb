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
require 'access_history_model'

describe AccessHistory do
  before(:each) do
    @history = AccessHistory.new(3)
  end

  it "should start with <start> text" do
    @history.prev.should eql('<start>')
  end

  it "should end with empty text and not wrap" do
    @history.next.should eql('')
    @history.next.should eql('')
  end

  it "supports push/prev" do
    @history.push('hello')
    @history.prev.should eql('hello')
  end

  it "should wrap on prev" do
    @history.push('xyzzy')
    @history.prev.should eql('xyzzy')
    @history.prev.should eql('<start>')
    @history.prev.should eql('xyzzy')
    @history.prev.should eql('<start>')
  end

  it "should keep only proscribed number of items" do
    (0..20).each { |x| @history.push(x.to_s)}
    @history.next.should eql('')
    @history.prev.should eql('20')
    @history.prev.should eql('19')
    @history.prev.should eql('<start>')
  end
end
