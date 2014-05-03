% Author: Bishwamoy Sinha Roy

function[] = test(serial_port)
%commands = struct{'straight', 0, 'left', 1, 'right', 2};
fopen(serial_port)
sensors1 = zeros(1, 20);
sensors2 = zeros(1, 20);
sensors3 = zeros(1, 20);
i = 1;
while( i < 300 )
    %sendToRover(0, 0, 0, serial_port);
    %pause(0.5);
    [messageGood, msgID, numOfSamples, Samples, Distances] = readFromRover(serial_port);
    if( msgID == 0 )
        sensors1(1, i) = bitor(bitshift(Samples(1, 1), 8), Samples(1,2));
        sensors2(1, i) = bitor(bitshift(Samples(1, 3), 8), Samples(1,4));
        sensors3(1, i) = bitor(bitshift(Samples(1, 5), 8), Samples(1,6));
        i = i + 1
    end
     if( i == 30 )
         graph(sensors1, sensors2, sensors3);
         fclose(serial_port);
         pause(1);         
         fopen(serial_port);
         i = 1;
     end
end
fclose(serial_port);
%graph(sensors1, sensors2, sensors3);
end
%end

function [] = graph(sensors1, sensors2, sensors3)
% sensors1 = (sensors1 .* 3.3) ./ 1024;
% sensors1 = sensors1 .^ 1.02;
% sensors1 = (22.4879) .* (1 ./ sensors1);

sensors2 = (sensors2 .* 3.3) ./ 1024;
sensors2 = sensors2 .^ 1.09;
sensors2 = (22.474) .* (1 ./ sensors2);

sensors3 = (sensors3 .* 3.3) ./ 1024;
sensors3 = sensors3 .^ 1.03;
sensors3 = (22.9116) .* (1 ./ sensors3);

% figure
% plot(sensors1, 'g');
% ylim([0 60]);
% xlabel('Samples');
% ylabel('Analog Voltage (V)');
% title('Top');
figure
plot(sensors2, 'r');
ylim([0 60]);
xlabel('Samples');
ylabel('Analog Voltage (V)');
title('SideTop');
figure
plot(sensors3, 'b');
ylim([0 60]);
xlabel('Samples');
ylabel('Analog Voltage (V)');
title('SideBottom');
end

function [messageGood, msgID, numOfSamples, Samples, Distances] = readFromRover(serial_port)
    %---------------------------------------------------------------
    %masks
    LSnibble_mask = uint8(15);%mask for the least sig nibble
    MSnibble_mask = uint8(240);%mask for the most sig nibble
    
    MSBytemask = uint16(65280);
    LSBytemask = uint16(255);
    %---------------------------------------------------------------
    

    while serial_port.BytesAvailable < 1 % wait for the buffer to receive atleast 1 byte (ID)  
        %i = i + 1
    end

%not enough bytes to signify even the smallest of messages
% if serial_port.BytesAvailable < 6  
%     fclose( serial_port );
%     messageGood = 0;
%     msgID = 0;
%     numOfSamples = 0;
%     Samples = zeros(1,1);
%     MotorRotations = zeros(1,1);
% end

    rollingSum = 0; %for the checksum

    firstByte = fread( serial_port, 1 ); % read the first byte that was sent (will contain length)

    rollingSum = rollingSum + firstByte;

    numSensorBytes = uint8(bitand(uint8(firstByte),uint8(LSnibble_mask),'uint8')); %get the number of bytes of sensor (least sign nibble)
    id_unshifted = bitand(uint8(firstByte),uint8(MSnibble_mask),'uint8');
    id = bitshift(id_unshifted,-4); %most sig nibble >> 4 is the id

    samp = zeros(numSensorBytes/2);
    dist = zeros(4);

%not enough bytes were read to signify the rest of the message
% if serial_port.BytesAvailable < numSensorBytes + 5  
%     fclose( serial_port );
%     messageGood = 0;
%     msgID = 0;
%     numOfSamples = 0;
%     Samples = zeros(1,1);
%     MotorRotations = zeros(1,1);    
% end

    [m, values_read] = fread( serial_port, double(numSensorBytes + 5) ); 
    msg = m';
    if(numSensorBytes ~= 0)
        samp = msg(1, 1:numSensorBytes);
    end

    dist = msg(1, numSensorBytes+1 : numSensorBytes+4);
    rollingSum = rollingSum + sum(msg(1, 1:numSensorBytes + 4));

    chkSum = msg(1, 5 + numSensorBytes);
    %message was good (checksum check passed)
    if( chkSum == uint8(bitand(rollingSum, LSBytemask)) )
        good = 1;
    else
        good = 0;
    end
    
    messageGood = good;
    msgID = id;
    numOfSamples = numSensorBytes/2;
    Samples = samp;
    Distances = dist;  
end

function [] = sendToRover(msgID, numOfCmds, Commands, serial_port)
    %---------------------------------------------------------------
    %masks
    LSnibble_mask = uint8(15);%mask for the least sig nibble
    MSnibble_mask = uint8(240);%mask for the most sig nibble
    
    MSBytemask = uint16(65280);
    LSBytemask = uint16(255);
    %---------------------------------------------------------------
    
    %make sure the things sent are uint8
    %should be 4 but no matlab data type for 4-bits  
    %(id > 255 will be defaulted to 255) = will also be picked up as an error by the ARM
    id = uint8(msgID); 
    num = uint8(numOfCmds);    
    cmds = uint8(Commands);
    
    bytes = 1;
    
    %package data into message
    message = uint8(zeros(1, 1 + 4*num + 1)); %each command is actually 4 bytes
    message(1,1) = bitor(bitshift(id, 4), bitand(num*4, LSnibble_mask)); % id << 4 | num*4 & 0x0F
    bytes = bytes + 1;
    index_cmd_begin = 1;
    index_cmd_end = 1;
    for i = 1:num
        index_cmd_begin = bytes;
        index_cmd_end = (index_cmd_begin + 3);
        cmd = zeros(1,4);
        cmd(1,1) = bitor(bitshift(cmds(i,1), 4), bitand(cmds(i,2), LSnibble_mask)); % cmdID << 4 | tId & 0xF
        cmd(1,2) = cmds(i,3); % turn Degress
        cmd(1,3) = cmds(i,4); % distance feet
        cmd(1,4) = cmds(i,5); % distance inches
        message(1, index_cmd_begin: index_cmd_end) = cmd(1, 1:4);
        bytes = index_cmd_end + 1;
    end
    
    rollingSum = sum(message);
    message(1, index_cmd_end + 1) = bitand(rollingSum, LSBytemask); % take the LSByte of the total
    fwrite(serial_port, message, 'uchar');
end
