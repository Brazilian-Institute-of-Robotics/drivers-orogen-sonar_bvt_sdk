#! /usr/bin/env ruby
# -*- coding: utf-8 -*-
# If you want to start the Microsoft Life Cam or the Gumstix camera e-CAM32
# you should use the corresponding ruby run-script. 

require 'orocos'
#require 'vizkit'
include Orocos
Orocos.initialize

Orocos.run "sonar_bvt_sdk::Task" => "bvt" do
    
    #Orocos.log_all

    bvt = Orocos.name_service.get 'bvt'
    #bvt = TaskContext.get 'bvt'
    Orocos.apply_conf_file(bvt, './ConfigFiles/bvt.yml', ['default'] )
    
    bvt.configure
    bvt.start
    
    loop do
        sleep 0.01
    end
end
