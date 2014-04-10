% Author: Bishwamoy Sinha Roy

function [] = Validator(serial_port)
warning off;

%getFromARM(serial_port);
%sendToARM(0, 3, [12, 12, 12], [50, 78], serial_port);

[rm, obst, st, fin, rov]  = xmlToMap();
distances = uint16(zeros(1,2)); % (right, left) rotations
constants = struct('msgID_sensorReq', uint8(0), 'msgID_sensorReq2', uint8(1), 'msgID_cmd', uint8(2), ...
    'cmdID_start', uint8(0), 'cmdID_stop', uint8(1), 'cmdID_slow', uint8(2), 'cmdID_fast', uint8(3), ...
    'tID_straight', uint8(0), 'tID_left', uint8(1), 'tID_right', uint8(2), 'rState_free', uint8(0), ...
    'rState_seenSt', uint8(2));

state = constants.rState_free;

% a struct to denote the state of the whole system - room with obstacles
% and the rover in the room
state = struct('Room', rm, 'Obstacles', obst, 'Start', st, 'Finish', fin, ...
    'Rover', rov, 'Distance', distances, 'constIDs', constants, 'rovState', state);

%recognize line segments that are possible obstacles to the rover (walls
%included)
possibleObstacles = zeros((length(state.Obstacles)/5)*4 + 4, 4);
%walls
possibleObstacles(1,1:4) = [0 0 state.Room.len 0]; %LS- x-axis
possibleObstacles(2,1:4) = [state.Room.len 0 state.Room.len state.Room.wid];
possibleObstacles(3,1:4) = [state.Room.len state.Room.wid 0 state.Room.wid];
possibleObstacles(4,1:4) = [0 state.Room.wid 0 0];
%obstacles
pos = 5;
path = zeros(600, 4); 
differentPathCmds = 1;
path(differentPathCmds,1:4) = [state.Rover.ref state.Rover.orientation 0];
for i = 1:(length(state.Obstacles)/5)
    obst_x = state.Obstacles(1, 1 + 5*(i-1));
    obst_y = state.Obstacles(1, 2 + 5*(i-1));
    obst_len = state.Obstacles(1, 3 + 5*(i-1));
    obst_wid = state.Obstacles(1, 4 + 5*(i-1));
    obst_orien = state.Obstacles(1, 5 + 5*(i-1));
    obp1 = convertCoordinate([-obst_len/2 -obst_wid/2], [obst_x obst_y], obst_orien);
    obp2 = convertCoordinate([obst_len/2 -obst_wid/2], [obst_x obst_y], obst_orien);
    obp3 = convertCoordinate([obst_len/2 obst_wid/2], [obst_x obst_y], obst_orien);
    obp4 = convertCoordinate([-obst_len/2 obst_wid/2], [obst_x obst_y], obst_orien);

    possibleObstacles(pos,1:4) = [obp1 obp2];
    possibleObstacles(pos + 1,1:4) = [obp2 obp3];
    possibleObstacles(pos + 2,1:4) = [obp3 obp4];
    possibleObstacles(pos + 3,1:4) = [obp4 obp1];
    pos = pos + 4;
end
%-----------------------------------------
%replyToArm(state, possibleObstacles, serial_port);
%obstHit(state, [48, 48], [52, 48], possibleObstacles, 0, state.Rover.ref);
%rfidModel([48, 48], [30, 48], state, 180);
% dToNinety = (45*pi/180)*state.Rover.wid;
% [newLoc, newO, right, left] = roverModel(state, dToNinety, -dToNinety, 3);
% state.Rover.ref = newLoc;
% state.Rover.orientation = newO;
%executeCmd([0 0 9 0], state, possibleObstacles, serial_port);
%le = sonarModel(30)
%-----------------------------------------
badMsgs = 0;
done = false;
draw(state, path(1:differentPathCmds, 1:4));
b = 0;
l = 1;
cf = [0 0 0 0];
cs1 = [0 0 0 0];
cs2 = [0 0 0 0];
fclose(serial_port);
fopen(serial_port);
% for j = 1:4
% [state, pth, ok] = executeCmd([0 0 1 0; 0 0 1 0; 0 0 1 0; 0 0 1 0; 33 90 0 0], state, possibleObstacles, serial_port);
%  s = size(pth);
%  lastEnd_rov = [path(differentPathCmds,4), 0];%end in rover coordinates
%  lastEnd = convertCoordinate(lastEnd_rov, path(differentPathCmds,1:2), path(differentPathCmds,3));
%  for i = 1:s(1)
%      if( lastEnd(1) == pth(i, 1) && lastEnd(2) == pth(i, 2) && path(differentPathCmds,3) == pth(i,3) )
%          path(differentPathCmds,4) = path(differentPathCmds,4) + pth(i, 4);
%      else
%          differentPathCmds = differentPathCmds + 1;
%          path(differentPathCmds,1:4) = pth(i, 1:4);
%      end
%  end
%  state = replyToArm(state, possibleObstacles, serial_port);
% end
draw(state, path(1:differentPathCmds, 1:4));
disp('wait');
pause(1);
disp('go!');
while done == false
    time1 = clock();
    [msgGood, ID, num, cmd] = getFromARM(serial_port);
    l = l + 1;
%     ID = 2;
%     msgGood = 1;
    if (msgGood == 1)        
        switch ID
            case 0
                state = replyToArm(state, possibleObstacles, serial_port);
                disp('rcvdSensor');
            case 1
                disp('yo');
            case 2
                [state, pth, ok] = executeCmd(cmd, state, possibleObstacles, serial_port);
                disp('rcvdCMD');
                s = size(pth);
                lastEnd_rov = [path(differentPathCmds,4), 0];%end in rover coordinates
                lastEnd = convertCoordinate(lastEnd_rov, path(differentPathCmds,1:2), path(differentPathCmds,3));
                for i = 1:s(1)
                    if( lastEnd(1) == pth(i, 1) && lastEnd(2) == pth(i, 2) && path(differentPathCmds,3) == pth(i,3) )
                        path(differentPathCmds,4) = path(differentPathCmds,4) + pth(i, 4);
                    else
                        differentPathCmds = differentPathCmds + 1;
                        path(differentPathCmds,1:4) = pth(i, 1:4);
                    end
                end
                if( ok == 0 )
                    done = true;
                end
            case 3
                done = true;
            otherwise
                disp('Problem with ID');
                badMsgs = badMsgs + 1;
        end
    else
        badMsgs = badMsgs + 1;
    end
    time2 = clock();
    elapsed = etime(time2, time1)
end
draw(state, path(1:differentPathCmds, 1:4));
fclose(serial_port);
end
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------
% Draw room on graph
function [] = draw(state, path)
    x = linspace(-10, state.Room.wid + 10, state.Room.wid+21);
    drawRect(state.Room.len/2, state.Room.wid/2, state.Room.wid, state.Room.wid, 0, 'w'); 
    
    hold on
    line([state.Finish.p1(1) state.Finish.p2(1)], [state.Finish.p1(2) state.Finish.p2(2)]);
    %finish line
    line([state.Finish.p1(1) state.Finish.p2(1)], [state.Finish.p1(2) state.Finish.p2(2)]);
    hold on
    %start line
    line([state.Start.p1(1) state.Start.p2(1)], [state.Start.p1(2) state.Start.p2(2)]);
    hold on
    %draw obstacles    
    for i = 1:(length(state.Obstacles)/5)
        obst_x = state.Obstacles(1, 1 + 5*(i-1));
        obst_y = state.Obstacles(1, 2 + 5*(i-1));
        obst_len = state.Obstacles(1, 3 + 5*(i-1));
        obst_wid = state.Obstacles(1, 4 + 5*(i-1));
        obst_orientation = state.Obstacles(1, 5 + 5*(i-1));
        drawRect(obst_x, obst_y, obst_len, obst_wid, obst_orientation,'r'); 
    end
    
    %draw rover
    rov_pt = state.Rover.ref;
    rov_len = state.Rover.len;
    rov_wid = state.Rover.wid;
    rov_orien = state.Rover.orientation;
    rov_sonp1 = convertCoordinate(state.Rover.sonp1, rov_pt, rov_orien);
    rov_sonp2 = convertCoordinate(state.Rover.sonp2, rov_pt, rov_orien);
    line([rov_sonp1(1) rov_sonp2(1)], [rov_sonp1(2) rov_sonp2(2)]);
    drawRect(rov_pt(1), rov_pt(2), rov_len, rov_wid, rov_orien, 'g');
    
    %draw the path
    s = size(path);
    for i = 1:s(1)
        lastEnd_rov = [path(i,4), 0];%end in rover coordinates
        lastEnd = convertCoordinate(lastEnd_rov, path(i,1:2), path(i,3));
        line([path(i,1) lastEnd(1)], [path(i,2) lastEnd(2)]);
    end
end

function [] = drawRect(x, y, len, wid, orien, color)
    p1 = convertCoordinate([-len/2 -wid/2], [x y], orien);
    p2 = convertCoordinate([len/2 -wid/2], [x y], orien);
    p3 = convertCoordinate([len/2 wid/2], [x y], orien);
    p4 = convertCoordinate([-len/2 wid/2], [x y], orien);
    x_p = [p1(1,1) p2(1,1) p3(1,1) p4(1,1)]; 
    y_p = [p1(1,2) p2(1,2) p3(1,2) p4(1,2)];
    fill(x_p,y_p,color);
end
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------
%MODELS
%return a sensor sample given a distance (front sensor)
function [sensorDat] = sonarModel(distance)
    if( distance > 254 )
        distance = 254;     
    end
    dist_int = double(0.0098*distance);
    sensorDat = dist_int * (1023/3.3);
end

%return a sensor sample given a distance (side sensor)
function [sensorDat] = ir_redModel(distance)
    if( distance > 60 )
        distance = 60;
    end
    dist_int = double(22.9116/distance);
    dist_int = (dist_int)^(1/1.03);
    sensorDat = dist_int * (1023/3.3);
end

function [sensorDat] = ir_blueModel(distance)
    if( distance > 60 )
        distance = 60;
    end
    dist_int = double(22.474/distance);
    dist_int = (dist_int)^(1/1.09);
    sensorDat = dist_int * (1023/3.3);
end

function [encounter, loc, dToObst] = rfidModel(curLoc, destLoc, state, orien)
    seen_start = false;
    seen_fin = false;
    start = state.Start;
    finish = state.Finish;
    dist = sqrt(state.Room.len^2 + state.Room.wid^2 );;
    locToStop = [0,0];
    %start detection
    [seen_start, dToSt] = detectHit(curLoc, destLoc, start.p1, start.p2);
    if( seen_start == true )        
        locToStop = convertCoordinate([dToSt 0], state.Rover.ref, orien);
        dist = dToSt;
        disp('start line detected!');
    end
    %finish detection
    [seen_fin, dToFin] = detectHit(curLoc, destLoc, finish.p1, finish.p2);
    if( seen_fin == true )        
        if( dToFin < dist )
            locToStop = convertCoordinate([dToFin 0], state.Rover.ref, orien);
            dist = dToFin;
        end
        disp('finish line detected!');
    end
    dToObst = dist;
    loc = locToStop;
    encounter = seen_start || seen_fin;    
end

function [newLoc, newO, rightTrav, leftTrav] = roverModel(st, distance_right, distance_left, timeTravelled)
    state = st;
    wid = state.Rover.wid;
    right_err = state.Rover.right_err;
    left_err = state.Rover.left_err;
    right = double(distance_right)*double(right_err);
    left = double(distance_left)*double(left_err);
    R =(wid/2)*((right + left)/(right-left));
    thetaTravelled = double((right - left)/wid);%this is radians...
    
    cur_pose = double([0, 0, 0]'); %move the center of the rover
    
    if( R ~= Inf && R ~= -Inf && distance_right ~= 0 && distance_left ~= 0 )    
        c = cos(thetaTravelled);
        s = sin(thetaTravelled);
        Transform = [c -s 0; s c 0; 0 0 1];
        ICC = double([cur_pose(1,1)-R*sin(cur_pose(3,1)), cur_pose(2,1)+R*cos(cur_pose(3,1))]);
        M = double([cur_pose(1,1) - ICC(1,1); cur_pose(2,1) - ICC(1,2); cur_pose(3,1)]);
        add = double([ICC(1,1); ICC(1,2); thetaTravelled]);
        Res = (Transform*M + add);
        Res(3,1) = Res(3,1)*180/pi;
        ptMap = convertCoordinate(Res', state.Rover.ref, state.Rover.orientation);
    else
        Res = zeros(3,1);
        ptMap = convertCoordinate([distance_right, 0], state.Rover.ref, state.Rover.orientation);
    end
    rightTrav = right;
    leftTrav = left;
    newLoc = ptMap(1,1:2);
    newO = Res(3,1);
end
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------
function [rotations] = convToRot(distance, radius)
    rot = double(distance)/(2*pi*double(radius));
    rotations = uint16(rot * 4000);
end

%figure the closest line segment from a ray
%input longest linsegment, base and dir of ray and all possible segments
function [willHit, newLoc, dToObst] = obstHit(state, curLoc, destLoc, possible_obst, orien, ref)       
    hit = false;
    stopLoc = curLoc;
    dist = sqrt(state.Room.len^2 + state.Room.wid^2 );
    dMove = 0;
    for i = 1:length(possible_obst)
        p1 = possible_obst(i, 1:2); %one end
        p2 = possible_obst(i, 3:4); %the other end
        [h, distToObst] = detectHit(curLoc, destLoc, p1, p2);
        if( h == true )            
            if( distToObst < dist )
                if( distToObst < 12 )
                    stopLoc = curLoc;
                    dMove = 0;
                else
                    stopLoc = convertCoordinate([distToObst - state.Rover.len/2 - 12, 0], ref, orien);
                    dMove = distToObst - state.Rover.len/2 - 12;
                end
                dist = distToObst;
                hit = h;
            end
        end
    end
    dToObst = dMove;
    newLoc = stopLoc;
    willHit = hit;
end

%figure out closest obstacle to each sesnor
function [closest] = obstDistRover(state, possible_obst)
    rov_ref = state.Rover.ref;    
    rov_orien = state.Rover.orientation;
    
    ir1p1 = convertCoordinate(state.Rover.ir1p1, rov_ref, rov_orien);
    ir1p2 = convertCoordinate(state.Rover.ir1p2, rov_ref, rov_orien);
    ir2p1 = convertCoordinate(state.Rover.ir2p1, rov_ref, rov_orien);
    ir2p2 = convertCoordinate(state.Rover.ir2p2, rov_ref, rov_orien);
    sonp1 = convertCoordinate(state.Rover.sonp1, rov_ref, rov_orien);
    sonp2 = convertCoordinate(state.Rover.sonp2, rov_ref, rov_orien);
    
    %lengths of the line segments used to denote the sensors
    d_ir1 = calcDist(ir1p1, ir1p2);
    d_ir2 = calcDist(ir2p1, ir2p2);
    d_son = calcDist(sonp1, sonp2);
    
    rov_ir1 = struct('base', ir1p1, 'dir', ir1p2);
    rov_ir2 = struct('base', ir2p1, 'dir', ir2p2);
    rov_son = struct('base', sonp1, 'dir', sonp2);
   
    init = sqrt(state.Room.len^2 + state.Room.wid^2 ); 
    
    closestFront = struct('obstacleN', 1, 'dist', init);
    closestSide1 = struct('obstacleN', 1, 'dist', init);
    closestSide2 = struct('obstacleN', 1, 'dist', init);
    
    for i = 1:length(possible_obst)
        p1 = possible_obst(i, 1:2); %one end
        p2 = possible_obst(i, 3:4); %the other end
        %front
        [distance_front, ignore] = distTo(p1, p2, rov_son.base, rov_son.dir);
        distance_front = distance_front * d_son;
        if( (distance_front < closestFront.dist && distance_front >= 0) )
            closestFront.obstacleN = i;
            closestFront.dist = distance_front;
        end
        %side
        [distance_side1, ignore] = distTo(p1, p2, rov_ir1.base, rov_ir1.dir);
        distance_side1 = distance_side1 * d_ir1;
        if( (distance_side1 < closestSide1.dist && distance_side1 >= 0) )
            closestSide1.obstacleN = i;
            closestSide1.dist = distance_side1;
        end
        %side
        [distance_side2, ignnore] = distTo(p1, p2, rov_ir2.base, rov_ir2.dir);
        distance_side2 = distance_side2 * d_ir2;
        if( (distance_side2 < closestSide2.dist && distance_side2 >= 0) )
            closestSide2.obstacleN = i;
            closestSide2.dist = distance_side2;
        end
    end
    closest = struct('Front', closestFront, 'SideTop', closestSide1, 'SideDown', closestSide2);
end

%return true if the path fron curLoc to destLoc will collide with
%line segment p1 -> p2
function [willHit, distToObst] = detectHit(curLoc, destLoc, p1, p2)
    hit = false;
    [distanceTo, where] = distTo(p1, p2, curLoc, destLoc);
    if( (distanceTo >= 0 && distanceTo <= 1) && (distanceTo ~= Inf || distanceTo ~= -Inf))
        hit = true;
    end;
    ptOnObst = convdistOnLSToPoint(where*calcDist(p1, p2), p1, p2);
    distToObst = calcDist(curLoc, ptOnObst);
    willHit = hit;
end

% figure out the distance to every line segment
% input the two points of the wall/obstacle and the sensor dir
% r1 -> r2 ray will hit p1 -> p2 line segment
function [distance, proportionHit] = distTo(p1, p2, r1, r2)
    % construct matrix
    M(1,1) = r2(1) - r1(1);
    M(1,2) = p1(1) - p2(1);
    M(2,1) = r2(2) - r1(2);
    M(2,2) = p1(2) - p2(2);

    % construct RHS
    rhs(1) = p1(1) - r1(1);
    rhs(2) = p1(2) - r1(2);

    % solve for intersection
    intersection = M\rhs';
    
    %proper intersection with LS
    dist = 0;
    if( intersection(2) <= 1 && intersection(2) >= 0 )
        dist = intersection(1);      
    else
        dist = Inf;
    end
    proportionHit = intersection(2);
    distance = dist;
end

function [point] = convdistOnLSToPoint(proportionHit, p1, p2)
   delX = p2(1) - p1(1);
   delY = p2(2) - p1(2);
   theta = 0;
   if( delX > 0 )
       theta = tan(delY/delX);
   else
       if( delY > 0 )
           theta = 90*pi/180;
       else
           theta = 270*pi/180;
       end
   end
   
   point = [proportionHit*cos(theta) + p1(1), proportionHit*sin(theta)+ p1(2)]; 
end

%convert from rover_coordinates to map_coordinates
function [m_point] = convertCoordinate(pointToConvert, reference, rotation)
    % Represent in homogenous coordinate
    p = double([pointToConvert(1), pointToConvert(2), 1]');
    
    % position of Rover in map coordinate system
    tx = reference(1);
    ty = reference(2);
    % orientation of Rover (clockwise rotation in degrees)
    R = rotation;

    % General Rover position
    RoverPos = [tx ty R];

    % Homogeneous Transformation Matrix
    c = cos(pi*RoverPos(3)/180);
    s = sin(pi*RoverPos(3)/180);
    Transform = [c -s RoverPos(1); s c RoverPos(2); 0 0 1];

    % Transform Sensor to Map coordinate system
    a = Transform*p;
    
    m_point = [a(1), a(2)];
end

% Arm communication higher level
% input comport and baudrate to send reply to a sensor request from the ARM
function [updatedState] = replyToArm(st, possible_obst, serial_port)
    state = st;
    %parameters
    numOfFrontSamples = 1; % design decision to store only 3 most recent samples
    numOfSideSamples = 1; % design decision to store only 2 most recent samples
    
    front = ones(1,numOfFrontSamples) .* 1023;
    side1 = ones(1,numOfSideSamples) .* 1023; %ir1
    side2 = ones(1,numOfSideSamples) .* 1023; %ir2
    id = state.rovState;
    
    closest = obstDistRover(st, possible_obst);
    
    for i = 1:numOfFrontSamples
        front(1, i) = sonarModel(closest.Front.dist);
        disp('Front inches = '); 
        disp(closest.Front.dist);
        disp('Front sensor = '); 
        disp(uint16(front(1,i))); 
    end
    
    for i = 1:numOfSideSamples
        side1(1, i) = ir_blueModel(closest.SideTop.dist);%top one is 
        disp('side_top inches = '); 
        disp(closest.SideTop.dist);
        disp('Side_top sensor = '); 
        disp(uint16(side1(1, i))); 
        side2(1, i) = ir_redModel(closest.SideDown.dist);
        disp('side_down inches = '); 
        disp(closest.SideDown.dist);
        disp('Side_down sensor = '); 
        disp(uint16(side2(1, i))); 
    end

    sendToARM(id, numOfFrontSamples + 2*numOfSideSamples, [front, side1, side2], [0,0], serial_port);
    state.Rotations = [0 0];
    updatedState = state;
end

function [dist] = calcDist(p1, p2)
    dist = sqrt( (p2(1) - p1(1))^2 + (p2(2)-p1(2))^2 ); 
end
% execute a command from the ARM and reply with an ACK/NACK
% input state is the condition of the current room
function [updated_state, path, ok] = executeCmd(commands, st, possible_obst, serial_port)
    state = st;
    sz = size(commands);
    num = sz(1);
    
    differentPathCmds = 1;
    pth = zeros(uint32(num/2), 4);
    pth(1,1:4) = [state.Rover.ref state.Rover.orientation 0]; 
    k = 1;   
    
    %cmd format = ([cmdID|tID, deg, feet, inches], []...)
    for i = 1:num%number of rows (each row is one command)
        rov_ref = state.Rover.ref;
        rov_len = state.Rover.len;
        rov_wid = state.Rover.wid;
        rov_orien = state.Rover.orientation;
        rov_wheelRad = state.Rover.radius;
        
        cmd = commands(i, 1:4);
        [id, turnID, degrees, dist] = parseCmd(cmd);
        newLoc = rov_ref;
        newO = rov_orien;
        rightTrav = 0;
        leftTrav = 0;
        degreesTurned = 0;
        distanceTrav = 0;
        check = 0;
        
        switch id
            case {st.constIDs.cmdID_start, st.constIDs.cmdID_slow, st.constIDs.cmdID_fast}
                switch(turnID)
                    case st.constIDs.tID_straight
                        %go straight
                        [newLoc, newO, rightTrav, leftTrav] = roverModel(state, dist, dist, 3);
                    case st.constIDs.tID_left
                        %left - Clock
                        disp('left');
                        dToDeg = ((double(degrees)*pi/180)/2)*rov_wid;
                        [newLoc, newO, rightTrav, leftTrav] = roverModel(state, dToDeg, -dToDeg, 3);
                    case st.constIDs.tID_right
                        %right - CounterClock
                        disp('right');
                        dToDeg = ((double(degrees)*pi/180)/2)*rov_wid;
                        [newLoc, newO, rightTrav, leftTrav] = roverModel(state, -dToDeg, dToDeg, 3);
                    otherwise
                        disp('incorrect turnID');
                end                
            case st.constIDs.cmdID_stop
                %stop
            otherwise
                disp('Unrecognized commands sent by ARM');
        end
        
        %check if newLoc clears all obstacles        
        [willHit, obstHitLoc, dToObst] = obstHit(state, rov_ref, newLoc, possible_obst, newO, rov_ref);        
        %check for start/finish hit
        [rfidResult, StFinLocToStop, dToSTFin] = rfidModel(rov_ref, newLoc, state, newO);
        dist = sqrt(state.Room.len^2 + state.Room.wid^2 );% init to longest possible distance
        if( willHit == true )
            if( dToObst < dist )
                disp('Obst Hit');
                dist = dToObst;
                [newLoc, newO, rightTrav, leftTrav] = roverModel(state, dToObst, dToObst, 3);
            end
        end
        if( rfidResult == true )
            if( dToSTFin < dist && state.rovState ~= state.constIDs.rState_seenSt )
                state.rovState = state.constIDs.rState_seenSt;
                dist = dToSTFin;                
                [newLoc, newO, rightTrav, leftTrav] = roverModel(state, dToSTFin, dToSTFin, 3);
            else
                if(state.rovState == state.constIDs.rState_seenSt)
                    state.rovState = state.constIDs.rState_free;
                end
            end
        end
        
        distanceTrav = calcDist(newLoc, rov_ref);
        degreesTurned = newO;
        
        %set new destination
        state.Rover.ref = newLoc;
        state.Rover.orientation = newO + state.Rover.orientation;
        disp('ref'); 
        disp(newLoc);
        disp('o');
        disp(state.Rover.orientation)
        %determin path
        if( state.Rover.orientation == pth(differentPathCmds, 3) )
            pth(differentPathCmds, 4) = pth(differentPathCmds, 4) + distanceTrav;
        else            
            differentPathCmds = differentPathCmds + 1;
            pth(differentPathCmds, 1:4) = [state.Rover.ref state.Rover.orientation 0];
        end
        
        %calculate rotations
        rotRight = convToRot(rightTrav,rov_wheelRad);
        rotLeft = convToRot(leftTrav,rov_wheelRad);
        state.Distance = uint16(state.Distance) + [rotRight, rotLeft];        
    end
    
    %send an ack
    sendToARM(3, 0, 0, [0 0 0 0], serial_port);
    
    ok = k;
    updated_state = state;
    path = pth(1:differentPathCmds, 1:4);
end
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------
%distance is in inches
function [cmdID, turnID, turnDegree, distance] = parseCmd(c)
    cmd = uint8(c);
    
    LSnibble_mask = 15;%mask for the least sig nibble
    MSnibble_mask = 240;%mask for the least sig nibble
    
    id_unshifted = bitand(uint8(cmd(1,1)),uint8(MSnibble_mask),'uint8');
    turnID = bitand(uint8(cmd(1,1)),uint8(LSnibble_mask),'uint8');
    cmdID = bitshift(id_unshifted,-4); %most sig nibble >> 4 is the id
    turnDegree = cmd(1,2);
    distance = uint32(cmd(1,3))*12 + uint32(cmd(1,4)); %convert to inches
end

% ARM comm formatting
% input the Com port
% ouput the message ID
% ouptut if message was proper ( check sum )
% ouput number of commands
% output the commands
function [messageGood, msgID, numOfCmds, commands] = getFromARM(serial_port)
LSnibble_mask = 15;%mask for the least sig nibble
MSnibble_mask = 240;%mask for the least sig nibble
MSBytemask = uint16(65280);
LSBytemask = uint16(255);%mask for the most sig nibble

while serial_port.BytesAvailable < 1 % wait for the buffer to receive atleast 2 bytes (count and ID)  
end

rollingSum = 0; %for the checksum

[firstByte, values_read] = fread( serial_port, 1 ); % read the first byte that was sent (will contain length)

rollingSum = rollingSum + firstByte;

numOfcmdBytes = uint8(bitand(uint8(firstByte),uint8(LSnibble_mask),'uint8')); %get the lenght (least sign nibble)
cmds = zeros(numOfcmdBytes/4, 4);
id_unshifted = bitand(uint8(firstByte),uint8(MSnibble_mask),'uint8');
id = bitshift(id_unshifted,-4); %most sig nibble >> 4 is the id
% wait for the COM port buffer to receive the appropriate number of commands and the checksum
while serial_port.BytesAvailable < numOfcmdBytes + 1 
end

[c, values_read] = fread( serial_port, double(numOfcmdBytes + 1) );
cmd = c';
l = int16((numOfcmdBytes/4)-1);
for i = 0:int16(numOfcmdBytes/4)-1
    cmds(i+1, 1:4) = cmd(1, 4*i + 1 : 4*(i+1));
end

rollingSum = rollingSum + sum(sum(cmds)); %first sum is of all collumns then total

chkSum = cmd(1, 1 + numOfcmdBytes);
%message was good (checksum check passed)
if( chkSum == uint8(bitand(rollingSum, LSBytemask)) )
    good = 1;
else
    good = 0;
end

messageGood = good;
msgID = id;
numOfCmds = numOfcmdBytes/4;
commands = cmds;
end

%input the msgID, number of samples to be sent
%input the samples - need to 16 bits (actually represent 10 bit sensor
%samples
%input Distance in inches
%This function will package the data and send according to the format
function [] = sendToARM(msgID, numOfSamples, Samples, Distances, serial_port)
    LSnibble_mask = uint8(15);%mask for the least sig nibble
    MSnibble_mask = uint8(240);%mask for the most sig nibble
    
    MSBytemask = uint16(65280);
    LSBytemask = uint16(255);%mask for the least sig byte
    
    %make sure the things sent are uint8
    %should be 4 but no matlab data type for 4-bits  
    %(id > 255 will be defaulted to 255) = will also be picked up as an error by the ARM
    id = uint8(msgID); 
    num = uint8(numOfSamples);   
    samp = uint16(Samples);
    
    distData_parsed = zeros(1,4);
    for i=0:1
        distData_parsed(1, 2*i + 1) = floor(Distances(1,i+1)/12); %feet
        distData_parsed(1, 2*i + 2) = mod(Distances(1,i+1),12); %inches
    end
    
    %package data into message
    message = uint8(zeros(1, 1 + num*2 + 4 + 1)); %4 bytes for motor stuff last byte for the checksum
    rollingSum = 0;
    message(1,1) = bitor(bitshift(id, 4), bitand(num*2, LSnibble_mask)); % id << 4 | num*2 & 0x0F
    for i=0:num-1
        % log the sample data into the message buffer
        %MSB of the sensor sample
        message(1,2+2*i) = uint8(bitshift(bitand(samp(1, i+1), MSBytemask), -8));
        %LSB of the sensor sample
        message(1,2+2*i + 1) = uint8(bitand(samp(1, i+1), LSBytemask));
    end
    index_motor_begin = 1 + num*2 + 1;
    index_motor_end = 1 + num*2 + 4;
    message(1, index_motor_begin: index_motor_end) = distData_parsed; %add the motor rotations
    rollingSum = sum(message);
    message(1, index_motor_end + 1) = bitand(rollingSum, LSBytemask); % take the LSByte of the total 
    message
    fwrite(serial_port, message, 'uchar');
    %fclose( serial_port );
end
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------
% XML reading
function [room, obstacles, start, finish, rover] = xmlToMap()
%following positions result from the element number in the xml file
room_child_pos = 6; %which element in the resulting struct will the room element result in
st_child_pos = 8; %which element in the resulting struct will the start element result in
fin_child_pos = 10; %which element in the resulting struct will the finish element result in
rover_child_pos = 12; %which element in the resulting struct will the rover element result in
rover_left_pos = 2;
rover_right_pos = 4;
rover_wheelRad_pos = 6;

%following positions result from the attribute number in the particular element
%room element
r_len_num = 1;
r_wid_num = 2;
%start element
st_p1_num = 1;
st_p2_num = 2;
%finish element
fin_p1_num = 1;
fin_p2_num = 2;
%rover elements
rov_ref_num = 7;
rov_len_num = 5;
rov_wid_num = 10;
rov_orien_num = 6;
rov_ir1P1_num = 1;
rov_ir1P2_num = 2;
rov_ir2P1_num = 3;
rov_ir2P2_num = 4;
rov_sonP1_num = 8;
rov_sonP2_num = 9;
%obstacle elements
obst_ref_num = 3;
obst_len_num = 1;
obst_wid_num = 4;
obst_orien_num = 2;

%read the xml file - should use a xsd to validate
xDoc = xmlread(fullfile('map.xml'));

try
   theStruct = parseChildNodes(xDoc);
catch
   error('Unable to parse XML file %s.',filename);
end

%length and width of the room
roomLength = double(str2num(theStruct.Children(room_child_pos).Attributes(r_len_num).Value));
roomWidth = double(str2num(theStruct.Children(room_child_pos).Attributes(r_wid_num).Value));
%x,y pos of start
st_p1 = double(str2num(theStruct.Children(st_child_pos).Attributes(st_p1_num).Value));
st_p2 = double(str2num(theStruct.Children(st_child_pos).Attributes(st_p2_num).Value));
%x,y pos of finish
fin_p1 = double(str2num(theStruct.Children(fin_child_pos).Attributes(fin_p1_num).Value));
fin_p2 = double(str2num(theStruct.Children(fin_child_pos).Attributes(fin_p2_num).Value));
%rover description
rov_ref = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_ref_num).Value));
rov_len = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_len_num).Value));
rov_wid = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_wid_num).Value));
rov_orien = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_orien_num).Value));
rov_ir1_p1 = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_ir1P1_num).Value));
rov_ir1_p2 = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_ir1P2_num).Value));
rov_ir2_p1 = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_ir2P1_num).Value));
rov_ir2_p2 = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_ir2P2_num).Value));
rov_son_p1 = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_sonP1_num).Value));
rov_son_p2 = double(str2num(theStruct.Children(rover_child_pos).Attributes(rov_sonP2_num).Value));
rov_left_err = double(str2num(theStruct.Children(rover_child_pos).Children(rover_left_pos).Children.Data));
rov_right_err = double(str2num(theStruct.Children(rover_child_pos).Children(rover_right_pos).Children.Data));
rov_wheelRad = double(str2num(theStruct.Children(rover_child_pos).Children(rover_wheelRad_pos).Children.Data));

%array to hold obatacle's info
%x,y,len,wid for each obstacle (so 5x the number of obstacles
numOfObst = int32(str2num(theStruct.Attributes(1).Value));
obst = zeros(1, 5 * numOfObst);
for i = 0:(numOfObst-1)
    obst(1, 1 + 5*i:2 + 5*i) = double(str2num(theStruct.Children(rover_child_pos + 2*(i+1)).Attributes(obst_ref_num).Value)); %ref pos of (i+1)th obstacle
    obst(1, 3 + 5*i) = double(str2num(theStruct.Children(rover_child_pos + 2*(i+1)).Attributes(obst_len_num).Value)); %y pos of (i+1)th obstacle
    obst(1, 4 + 5*i) = double(str2num(theStruct.Children(rover_child_pos + 2*(i+1)).Attributes(obst_wid_num).Value)); %length of (i+1)th obstacle
    obst(1, 5 + 5*i) = double(str2num(theStruct.Children(rover_child_pos + 2*(i+1)).Attributes(obst_orien_num).Value)); %width of (i+1)th obstacle    
end

%output the necessary variables
room = struct('len', roomLength, 'wid', roomWidth);
obstacles = obst;
start = struct('p1', st_p1, 'p2', st_p2);
finish = struct('p1', fin_p1, 'p2', fin_p2);
rover = struct('ref', rov_ref, 'len', rov_len, 'wid', rov_wid, 'orientation', ...
    rov_orien, 'ir1p1', rov_ir1_p1, 'ir1p2', rov_ir1_p2, 'ir2p1', rov_ir2_p1, ...
    'ir2p2', rov_ir2_p2, 'sonp1', rov_son_p1, 'sonp2', rov_son_p2, 'right_err',...
    rov_right_err, 'left_err', rov_left_err, 'radius', rov_wheelRad); 
end

% ----- Local function PARSECHILDNODES -----
function children = parseChildNodes(theNode)
% Recurse over node children.
children = [];
if theNode.hasChildNodes
   childNodes = theNode.getChildNodes;
   numChildNodes = childNodes.getLength;
   allocCell = cell(1, numChildNodes);

   children = struct(             ...
      'Name', allocCell, 'Attributes', allocCell,    ...
      'Data', allocCell, 'Children', allocCell);

    for count = 1:numChildNodes
        theChild = childNodes.item(count-1);
        children(count) = makeStructFromNode(theChild);
    end
end
end

% ----- Local function MAKESTRUCTFROMNODE -----
function nodeStruct = makeStructFromNode(theNode)
% Create structure of node info.

nodeStruct = struct(                        ...
   'Name', char(theNode.getNodeName),       ...
   'Attributes', parseAttributes(theNode),  ...
   'Data', '',                              ...
   'Children', parseChildNodes(theNode));

if any(strcmp(methods(theNode), 'getData'))
   nodeStruct.Data = char(theNode.getData); 
else
   nodeStruct.Data = '';
end
end

% ----- Local function PARSEATTRIBUTES -----
function attributes = parseAttributes(theNode)
% Create attributes structure.

attributes = [];
if theNode.hasAttributes
   theAttributes = theNode.getAttributes;
   numAttributes = theAttributes.getLength;
   allocCell = cell(1, numAttributes);
   attributes = struct('Name', allocCell, 'Value', ...
                       allocCell);

   for count = 1:numAttributes
      attrib = theAttributes.item(count-1);
      attributes(count).Name = char(attrib.getName);
      attributes(count).Value = char(attrib.getValue);
   end
end
end
%--------------------------------------------------------------------------
%--------------------------------------------------------------------------