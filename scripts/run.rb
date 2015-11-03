require 'orocos'
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
