; Simple Hello World
    .ORIG x3000
    LEA R0, HW
    PUTS
    HALT
HW  .STRINGZ "Hello World!"
    .END
