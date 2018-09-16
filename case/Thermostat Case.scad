use <C:\Users\Public\Documents\Prusa3D\Objects\path_extrude.scad>;

$fn = 20;

boxwidth = 90;
pcbLength = 80;
pcbWidth = 20;
pcbHeight = 1.6;

ledLength = 9;
ledDiameter = 2.5;
buttonsSpacing = 24.5;
ledSpacing = 24;
waveSpacing = 24;

CircuitBoard();
BoxBottomHollowed();
Radiator();
TopCover();


module Radiator() {
    difference() {
        for (x = [-3:1:3])
            translate([x * 15-5, 0, 0])
                RadiatorElement();
        TopCoverFull(1.05);
        CircuitBoard();
        UsbCutout();
        ComponentsCutout();
        DisplayCutout();
    }
}

module HeatWave(objScale) {
    w = 10*objScale;
    h = 10*objScale;
    l = 360;
    m = 3.5;
        
    step = 16;//4;
    C = 360;

    myPoints = [ 
        [ 0, 0 ],
        [ 0, w ],
        [ h, w ],
        [ h, 0 ],
    ];
    myPath = [ for(t = [-C/4:step:C+C/4]) [0,t,m*sin(t+180)] ];
    translate([0, 0, 0]) {
        scale([1,1,1/10]) {
            rotate([90,0,90]) {
                difference() {
                    path_extrude(points=myPoints, path=myPath);
                    // cut off the ends to have straight edges
                    translate([-5,-100,-h-10]) cube([h+10, 100, 50]);
                    translate([-5,l,-h-10]) cube([h+10, 100, 50]);
                }
            }
        }
    }
}

blockHeight = 8;
blockWidth = 10.8;
module TopCoverFull(objScale) {
    // support block with holes for LEDs
    translate([0, pcbHeight+5, 2*pcbWidth-(blockHeight*objScale/2)+12]) {
        cube([60*objScale, blockWidth*objScale, blockHeight*objScale], center=true);
        for (x = [-waveSpacing:waveSpacing:waveSpacing])
            translate([x + blockWidth*objScale/2, -blockWidth*objScale/2, blockHeight*objScale/2])
                HeatWave(objScale);
    }
    // button pads
    for (x = [-buttonsSpacing/2:buttonsSpacing:buttonsSpacing/2])
        translate([x, pcbHeight+2, 2*pcbWidth-(blockHeight*objScale/1)+12])
            color("Red") cylinder(2,2.5,2.5, center=true);
}


module TopCover() {
    difference() {
        TopCoverFull(1);
        for (x = [-ledSpacing:ledSpacing:ledSpacing])
            translate([x, 6.4, pcbWidth*2+7.5])
                scale(1.1) Led();
    }
}

module BoxBottomHollowed() {
    difference() {
        BoxBottom();
        CircuitBoard();
        UsbCutout();
        ComponentsCutout();
        DisplayCutout();
    }
}
module BoxBottom() {
    translate([0,10,0])
        cube([boxwidth,40,2], center=true);
    translate([0,0,2])
        cube([boxwidth-7,0,4], center=true);
}
module DisplayCutout() {
    translate([0,-10,18])
        cube([26,20,20], center=true);
}

module ComponentsCutout() {
    translate([0,2,pcbWidth+1])
        cube([pcbLength-6,13,pcbWidth*2-2], center=true);
}

module UsbCutout() {
    translate([pcbLength/2-1,2,8])
        cube([19,11,10]);
}

module CircuitBoard() {
    translate([0,0,pcbWidth * 1.5])
        PCB();
    translate([0,0,pcbWidth / 2])
        PCB();
    for (x = [-buttonsSpacing/2:buttonsSpacing:buttonsSpacing/2])
        translate([x,pcbHeight/2+3,pcbWidth*2-2])
            PushSwitch();
    translate([-3.5,pcbHeight/2,5])
        Zuno();
    translate([-pcbLength/2,pcbHeight/2,5])
        Sensor();
    translate([13.5,-1,4.5])
        rotate([0,0,180])
            Display();
    for (x = [-ledSpacing:ledSpacing:ledSpacing])
        translate([x, 6.4, pcbWidth*2+7.5])
            Led();
}


module Zuno() {
    color("white")
    cube([40,4,20]);
    translate([35,4,4])
        color("gray")
        cube([5,2.5,8]);
}

module Sensor() {
    cube([8,21,15]);
    difference() {
        translate([6,21,0])
            cube([2,6,15]);
        translate([7,24,7.5])
            rotate([0,90,0])
                color("white")
                cylinder(4,1.5,1.5, center=true);
    }
}

module Led() {
    color("green")
        cylinder(ledLength, ledDiameter, ledDiameter, center=true);
}

module Display() {
    import("display.stl", convexity=3);
}

module PCB() {
    d = 0.5;
    color("blue")
    difference() {
        cube([pcbLength,pcbHeight,pcbWidth], center=true);
        for (y = [3.75:2.5:pcbWidth-2])
            for (x = [6.25:2.5:pcbLength-5])
                translate([-pcbLength/2+x,0,-pcbWidth/2+y])
                    rotate([90,0,0])
                        cylinder(pcbHeight+1,d,d,center=true);
    }
}

module PushSwitch() {
    axisDiam = 1.65;
    axisLen = 3;
    footDiam = 0.25;
    footLen = 4;
    bodyHeight = 4;
    bodyWidth = 6;
    coverHeight = 0.3;
    feetSep = 5.5;
    // body
    cube([bodyWidth, bodyWidth ,bodyHeight], center=true);
    // axis
    color("black")
    translate([0, 0, axisLen/2 + bodyHeight/2])
        cylinder(axisLen, axisDiam, axisDiam ,center=true);
    // freedom of movement
    color("gray")
    translate([0, 0, axisLen + bodyHeight/2 + coverHeight/2])
        cylinder(coverHeight, axisDiam, axisDiam, center=true);
    // feet
    color("gray")
    translate([footLen/2+footDiam,0,-bodyHeight])
        cylinder(footLen, footDiam, footDiam, center=true);
    translate([-feetSep/2+footDiam,0,-bodyHeight])
        cylinder(footLen, footDiam, footDiam, ,center=true);
}

module RadiatorElement() {
    elementWidth = 5;
    elementHeight = 25;
    elementDepth = 40;
    rotate([90,0,0])
    translate([elementWidth, elementHeight, -10])
    hull() {
        translate([0,-elementHeight, 0])
            cylinder(elementDepth, elementWidth, elementWidth, center=true);
        translate([0, elementHeight, 0])
            cylinder(elementDepth, elementWidth, elementWidth, center=true);
    }
}