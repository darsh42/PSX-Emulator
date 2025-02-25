# cpu debugging features

define FUNCT
    printf("%x\n", (($(arg0) >>  0) & 0x3F))
end
define SHAMT    
    hex (($(arg0) >>  6) & 0x1F)
end
define RD       
    hex (($(arg0) >> 11) & 0x1F)
end
define RT       
    hex (($(arg0) >> 16) & 0x1F)
end
define RS       
    hex (($(arg0) >> 21) & 0x1F)
end
define OP       
    hex (($arg0 >> 26) & 0x3F)
end
define TARGET    
    hex ($(arg0) & ((1 << 26) - 1))
end
define IMM16     
    hex ($(arg0) & ((1 << 16) - 1))
end
define IMM25     
    hex ($(arg0) & ((1 << 25) - 1))
end

define RELATIVE  
    hex ($(arg0) & ((1 << 16) - 1))
end
