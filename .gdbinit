# cpu debugging features

define FUNCT
    hex (cpu.cir >>  0) & 0x3F
end
define SHAMT    
    hex ((cpu.cir >>  6) & 0x1F)
end
define RD       
    hex ((cpu.cir >> 11) & 0x1F)
end
define RT       
    hex ((cpu.cir >> 16) & 0x1F)
end
define RS       
    hex ((cpu.cir >> 21) & 0x1F)
end
define OP       
    hex eval "(($arg0 >> 26) & 0x3F)"
end
define TARGET    
    hex (cpu.cir & ((1 << 26) - 1))
end
define IMM16     
    hex (cpu.cir & ((1 << 16) - 1))
end
define IMM25     
    hex (cpu.cir & ((1 << 25) - 1))
end

define RELATIVE  
    hex (cpu.cir & ((1 << 16) - 1))
end
