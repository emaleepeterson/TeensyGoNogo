classdef TeensyGoNogo < handle

    % User modifieable properties:
    properties
        debugLevel = 1
    end
    % ---- do not change properties below this line ----


    % Properties that shouldn't be saved to disk
    % Must be *both* Transient and protected
    properties % (Access=protected, Transient=true)
        figureH = []
        uicontrolH = []
        lh
        figure_H
        figure_W

        logFile = []
        logFolder = 'C:\Data\teensy';
        loggingStartTime
        behviorData = []
        odorTypeList = []
        accuracy = []
        trialIndex;
        stimDurations = [50, 100, 200, 500, 1000, 2000]; 
        lookBackCounter
        currentDelay
        n_trials_since_switch
        isRunning = false
    end

    properties
        params = []
    end
    properties (SetObservable=true)
        className = 'TeensyGoNogo';
        classVersion = 0.3;
        user = [] % user parameters
        figurePosition = []
        figureSpacing = [];
    end

    properties %(Access=protected, Transient=true)
        numParams
        Arduino = []
    end

    methods
        function self = TeensyGoNogo()
            % creates the "TeensyGoNogo object"
            fprintf('New TeensyGoNogo Object.\n');

            % set up Arduino connection
            baud = 9600;
            self.Arduino = ArduinoConnection(@(msg) self.interpretArduinoMessage(msg), baud);

            % set up figure spacing
            self.figureSpacing.outer_buffer_H = 5;
            self.figureSpacing.outer_buffer_W = 5;
            self.figureSpacing.text_H = 20;
            self.figureSpacing.name_W = 150;
            self.figureSpacing.edit_W = 100;
            self.figureSpacing.units_W = 60;
            self.figureSpacing.buffer_H = 5;
            self.figureSpacing.buffer_W = 5;
            fs = self.figureSpacing;
            self.figureSpacing.list_W = fs.name_W + fs.edit_W + fs.units_W;
            fs = self.figureSpacing;
            self.figureSpacing.panel_W = fs.list_W + 2*fs.buffer_W;

            self.setupParams();
            % make figure.....
            fprintf('Making Figure.\n')
            self.makeFig();
            self.lh = addlistener(self,'user','PostSet', @(e,s) self.updateFigure());
            % self.lh = addlistener(self,'figurePosition','PostSet', @(e,s) self.updateFigurePosition());

            % link parameters between Teensy & Matlab
            pNames = fieldnames(self.user);
            for pNum = 1:numel(pNames)
                pName = pNames{pNum};
                self.setParam(pName, self.user.(pName));
            end

        end

        function delete(self)
            % Called when there are no more references to the object, just
            % prior to its memory being freed.
            fprintf('An instance of TeensyGoNogo is being deleted.\n');
            delete(self.figureH)
            fclose(self.Arduino) % this should close & delete the serial connection
        end

    end % methods

    methods % Arduino commands
        function setParam(self, paramName, val)
            %TODO: special cases for multiple rewarded odor pins
            if strcmp(paramName, 'pStim')
                val = round(val*1000);
            end
            self.Arduino.writeMessage(paramName, val, 0);

        end
    end % Arduino commands

    methods
%         function acc = calculateAccuracy(self)
%             if (length(self.accuracy) < self.user.numLookBack)
%                 acc = mean(self.accuracy);
%             else
%                 lastN = self.accuracy(end-self.user.numLookBack+1:end);
%                 acc = mean(lastN);
%             end
%         end
        
         function accuracy = getAccuracy(self, look_back)
            start_idx = self.trialIndex - look_back + 1;
%             self.debugOut(self.accuracy(self.odorTypeList(start_idx:self.trialNum)==1),1);
            accuracy = sum(self.accuracy(self.odorTypeList(start_idx:self.trialIndex)==1)) / ...
                sum(self.odorTypeList(start_idx:self.trialIndex)==1);
         end
         
         function updateDelay(self)
            curr_idx = find(self.stimDurations > self.currentDelay, 1, 'first');
            if isempty(curr_idx)
                self.debugOut('Current delay not in list. Using first delay',1);
                curr_idx = 1;
            end
            if curr_idx < length(self.stimDurations)
                self.currentDelay = self.stimDurations(curr_idx);
                self.setParam('stimulusDuration_ms', self.currentDelay);
                fprintf(['New stimulus duration: ' num2str(self.currentDelay) ' ms\n']);
            end
        end
        
                
        function debugOut(self, message, debugLevel)
            if nargin < 3
                debugLevel = 1;
            end
            if debugLevel <= self.debugLevel
                fprintf('%s\n',message)
            end
        end

        function runSession(self)
            self.disableParameterEditing();
            self.isRunning = true;

            self.setupLogging('TestBehavior'); % TODO: get this from a user-entered field
            pNames = fieldnames(self.user);
            for pName = pNames'
                pName = pName{1}; % convert from cell to char
                self.logValue(pName, self.user.(pName));
            end
            self.odorTypeList = zeros(1, self.user.numTrials);
            self.accuracy = zeros(1, self.user.numTrials);
            self.n_trials_since_switch = 0;
            self.currentDelay = self.user.stimulusDuration_ms;

            self.Arduino.writeMessage('run',0,0);
        end

        function stopSession(self) % initiate the end of a session
            self.Arduino.writeMessage('stop',0,0);
        end

        function sessionHasStopped(self) % what to do when the Teensy indicates it has stopped
            self.isRunning = false;
            self.enableParameterEditing();
            pause(0.5); % wait for final data to be logged
            fclose(self.logFile)
            self.logFile = [];
        end

        function setupLogging(self, logName)
            baseName = [self.logFolder, filesep,logName, '_', char(datetime,'yyyy-MM-dd'), '_'];
            fileCounter = 1;
            fName = [baseName, num2str(fileCounter, '%03d'), '.csv'];
            while (exist(fName, 'file'))
                fileCounter = fileCounter + 1;
                fName = [baseName, num2str(fileCounter, '%03d'), '.csv'];
            end
            [FileName,PathName] = uiputfile(fName,'Save log file' );
            fName = fullfile(PathName,FileName);
            self.logFile = fopen(fName, 'w');
            self.loggingStartTime = clock;
            self.logValue('StartTime', char(datetime(self.loggingStartTime)));
        end

        function logValue(self, valName, value)
            if self.logFile
                c = clock;
                secondsElapsed = etime(c, self.loggingStartTime);
                if isnumeric(value)
                    value = num2str(value);
                end
                fprintf(self.logFile, '%.4f,%s,%s\n', secondsElapsed , valName, value);
            end
        end

        function logEvent(self, eventName)
            self.logValue(eventName, '')
        end

        %% Parameter Setup
        function setupParams(self)
            params = [];

            p.name = 'numTrials';
            p.val = 100;
            p.units = '';
            p.min = 1;
            p.max = inf;
            p.step = 1;
            p.desc = 'Number of stimulus trials.';
            params = [params, p];

            p.name = 'pStim';
            p.val = 1;
            p.units = '';
            p.min = 0;
            p.max = 1;
            p.step = 0.001;
            p.desc = 'Probability of a reward trial.';
            params = [params, p];

            p.name = 'maxRunLength';
            p.val = 0;
            p.units = '';
            p.min = 0;
            p.max = Inf;
            p.step = 1;
            p.desc = 'Maximum run length of consecutive Go or No-go trials. (0 for no restrictions)';
            params = [params, p];

            p.name = 'numRewardedOdors';
            p.val = 4;
            p.units = '';
            p.min = 1;
            p.max = 5;
            p.step = 1;
            p.desc = 'Number of Go-odor concentrations. (1 for regular Go-NoGo; >1 for threshold tests.)';
            params = [params, p];

            p.name = 'stimulusDuration_ms';
            p.val = self.stimDurations(1);
            p.units = 'ms';
            p.min = 0;
            p.max = inf;
            p.step = 0.01;
            p.desc = 'Duration of stimulus.';
            params = [params, p];

            p.name = 'numLickBins';
            p.val = 4;
            p.units = 'bins';
            p.min = 1;
            p.max = inf;
            p.step = 1;
            p.desc = 'Number of time bins within stimulus delivery.';
            params = [params, p];

            p.name = 'minLickBins';
            p.val = 0;
            p.units = 'bins';
            p.min = 0;
            p.max = inf;
            p.step = 1;
            p.desc = 'Minumum number of bins with licks to trigger reward.';
            params = [params, p];


            p.name = 'rewardDuration_ms';
            p.val = 100;
            p.units = 'ms';
            p.min = 0;
            p.max = inf;
            p.step = 1;
            p.desc = 'Duration of reward delivery (valve opening).';
            params = [params, p];

            p.name = 'interTrialDuration_ms';
            p.val = 1000;
            p.units = 'ms';
            p.min = 0;
            p.max = inf;
            p.step = 0.01;
            p.desc = 'Time after end of odor before next trial can start.';
            params = [params, p];

            p.name = 'nullPenalty_ms';
            p.val = 5000;
            p.units = 'ms';
            p.min = 0;
            p.max = inf;
            p.step = 0.01;
            p.desc = 'Duration of penalty for licking during Null trials.';
            params = [params, p];

            p.name = 'pauseLickingDelay_ms';
            p.val = 1000;
            p.units = 'ms';
            p.min = 0;
            p.max = inf;
            p.step = 0.01;
            p.desc = 'Duration of time when mouse cannot lick during ITI';
            params = [params, p];

            p.name = 'lickPenaltyThresh';
            p.val = 1;
            p.units = '';
            p.min = 0;
            p.max = inf;
            p.step = 1;
            p.desc = 'Max number of licks during ITI before ITI is reset';
            params = [params, p];
            
            p.name = 'numLookBack';
            p.val = 2;
            p.units = '';
            p.min = 1;
            p.max = inf;
            p.step = 1;
            p.desc = 'Number of trials over which to calculate moving average';
            params = [params, p];
            
            p.name = 'accThresh';
            p.val = 0.7;
            p.units = '';
            p.min = 0;
            p.max = 1;
            p.step = 0.1;
            p.desc = 'Threshold for increasing stimulus duration';
            params = [params, p];
            

            % KEEP DEFAULT PIN VALUES FOR NOW...

            % p.name = 'beambreakPin';
            % p.name = 'lickPin';
            % p.name = 'LEDPin';
            % p.name = 'rewardPin';
            % p.name = 'blankOdorPin';
            % p.name = 'unrewardedOdorPin';
            % p.name = 'rewardedOdorPin_1';
            % p.name = 'rewardedOdorPin_2';
            % p.name = 'rewardedOdorPin_3';
            % p.name = 'rewardedOdorPin_4';
            % p.name = 'rewardedOdorPin_5';
            % p.val = 5;
            % p.units = '';
            % p.min = 0;
            % p.max = 23;
            % p.step = 1;
            % p.desc = 'Teensy pin number for beambreak.';
            % params = [params, p];

            %numParams = numel(params);

            self.params = params;
            self.user = [];
            for p=params
                self.user.(p.name) = p.val;
            end
        end %setupParams()

        function makeFig(self)
            backgroundColor = [.94 .94 .94];
            figH=figure('Units','Pixels', ...
                                    'WindowStyle', 'normal', ...
                                    'Color',backgroundColor, ...
                                    'Visible','off', ...
                                    'HandleVisibility','off', ...
                                    'DockControls','off', ...
                                    'Tag','gonogoBehavior', ...
                                    'Resize','off', ...
                                    'Name','Go-NoGo Behavior Protocol', ...
                                    'MenuBar','none', ...
                                    ... %'DockControls','off', ...
                                    'NumberTitle','off', ...
                                    ... % 'SizeChangedFcn', @(s,e)(fprintf('Size Changed\n')));
                                    'CloseRequestFcn',@(source,event)(self.delete()));
                                    %'WindowButtonUpFcn', @(s,e)(fprintf('MouseButtonUp')), ...

            fs = self.figureSpacing;

            self.figureH = figH;

            paramPanelH = self.makeParametersPanel(figH);
            pos = get(paramPanelH,'Position');
            paramPanel_W = pos(3);
            paramPanel_H = pos(4);
            set(paramPanelH,'Position',[fs.outer_buffer_W, ...
                                        fs.outer_buffer_H, ...
                                        paramPanel_W, ...
                                        paramPanel_H]);


            % buttons:
            controlPanelH = self.makeControlPanel(figH);
            pos = get(controlPanelH,'Position');
            controlPanel_W = pos(3);
            controlPanel_H = pos(4);
            set(controlPanelH,'Position',[fs.outer_buffer_W, ...
                                        fs.outer_buffer_H+paramPanel_H, ...
                                        controlPanel_W, ...
                                        controlPanel_H]);


            % Message panel
            messagePanelH = self.makeMessagePanel(figH);
            pos = get(messagePanelH,'Position');
            messagePanel_W = pos(3);
            messagePanel_H = pos(4);
            set(messagePanelH,'Position',[fs.outer_buffer_W, ...
                                        fs.outer_buffer_H+paramPanel_H+controlPanel_H, ...
                                        messagePanel_W, ...
                                        messagePanel_H]);


            self.figure_W = paramPanel_W + (fs.outer_buffer_W * 2);
            % self.figure_H = paramPanel_H + (fs.outer_buffer_H * 5) + fs.text_H*12.5;
            self.figure_H = paramPanel_H + messagePanel_H + controlPanel_H + (fs.outer_buffer_H * 5);
            leftPos = 100;
            bottomPos = 100;
            set(figH, 'Position', [leftPos, bottomPos, self.figure_W, self.figure_H])

            figH.Visible = 'on';
        end

        function messagePanel = makeMessagePanel(self, figH)
            self.uicontrolH.MessagePanel = ...
                uipanel('Parent',figH, ...
                        'Units','pixels',...
                        'Title','Messages');

            messagePanel = self.uicontrolH.MessagePanel;
            self.uicontrolH.MessageList = ...
                    uicontrol('Parent',messagePanel, ...
                                ...%'style','edit',...
                                'style','list',...
                                'Enable', 'inactive',...
                                'units','pix',...
                                'min',0,'max',2,...  % This is the key to multiline edits.
                                'string',{},...
                                'horizontalalign','left',...
                                'fontsize',11);

            fs = self.figureSpacing;

            set(self.uicontrolH.MessageList, ...
                    'Position', [fs.buffer_W , ...
                                fs.buffer_H, ...
                                fs.list_W, ...
                                fs.text_H*10]);

            set(self.uicontrolH.MessagePanel, ...
                    'Position', [0 , ...
                                0, ...
                                ... %fs.list_W + 2*fs.buffer_W, ...
                                fs.panel_W, ...
                                fs.text_H*10  + 4*fs.buffer_H]);

        end


        function controlPanel = makeControlPanel(self, figH)
            self.uicontrolH.ControlPanel = ...
                uipanel('Parent',figH, ...
                        'Units','pixels',...
                        'Title','Controls');

            controlPanel = self.uicontrolH.ControlPanel;

            fs = self.figureSpacing;

            self.uicontrolH.RewardButton = ...
                    uicontrol('Parent',controlPanel, ...
                              'Style','pushbutton', ...
                              'String','Deliver Reward', ...
                              'Callback', @(source,event)(self.rewardButtonAction()));
            set(self.uicontrolH.RewardButton, ...
                    'Position', [fs.buffer_W, ...
                                fs.buffer_H, ...
                                fs.name_W, ...
                                fs.text_H*1.25]);

            self.uicontrolH.MultiRewardButton = ...
                    uicontrol('Parent',controlPanel, ...
                              'Style','pushbutton', ...
                              'String','Deliver 20 Rewards', ...
                              'Callback', @(source,event)(self.multiRewardButtonAction()));
            set(self.uicontrolH.MultiRewardButton, ...
                    'Position', [fs.buffer_W*2 + fs.name_W, ...
                                fs.buffer_H, ...
                                fs.name_W, ...
                                fs.text_H*1.25]);

            self.uicontrolH.RunSessionButton = ...
                    uicontrol('Parent',controlPanel, ...
                              'Style','pushbutton', ...
                              'ForegroundColor',[0 0.5 0], ... % dark green
                              'FontWeight','bold', ...
                              'String','RUN SESSION', ...
                              'Callback', @(source,event)(self.runSessionButtonAction()));
            set(self.uicontrolH.RunSessionButton, ...
                    'Position', [fs.buffer_W, ...
                                fs.buffer_H + fs.text_H*1.25, ...
                                fs.name_W, ...
                                fs.text_H*1.25]);

            self.uicontrolH.StopSessionButton = ...
                    uicontrol('Parent',controlPanel, ...
                              'Style','pushbutton', ...
                              'ForegroundColor','red', ...
                              'FontWeight','bold', ...
                              'String','STOP', ...
                              'Callback', @(source,event)(self.stopSessionButtonAction()));
            set(self.uicontrolH.StopSessionButton, ...
                    'Position', [fs.buffer_W*2 + fs.name_W, ...
                                fs.buffer_H + fs.text_H*1.25, ...
                                fs.name_W, ...
                                fs.text_H*1.25]);

            set(self.uicontrolH.ControlPanel, ...
                    'Position', [0 , ...
                                0, ...
                                ... %fs.name_W*2 + 3*fs.buffer_W, ...
                                fs.panel_W, ...
                                fs.text_H*2.5  + 3*fs.buffer_H]);

        end



        function paramPanel = makeParametersPanel(self, figH)
            self.uicontrolH.ParametersPanel = ...
                uipanel('Parent',figH, ...
                        'Units','pixels',...
                        'Title','Parameters');

            paramPanel = self.uicontrolH.ParametersPanel;
            numParams = numel(self.params);
            self.numParams = numParams;
            ii = 0;
            for p=self.params
                ii = ii + 1;
                self.uicontrolH.ParameterText(ii) = ...
                    uicontrol('Parent',self.uicontrolH.ParametersPanel, ...
                                 'Style','text', ...
                                 'HorizontalAlignment','right', ...
                                 'String',p.name, ...
                                 'TooltipString',p.desc);
                if strcmp(p.units, 'Bool') % Boolean checkboxes
                    self.uicontrolH.ParameterEdit(ii) = ...
                        uicontrol('Parent',self.uicontrolH.ParametersPanel, ...
                                  'HorizontalAlignment','right', ...
                                  'Style', 'checkbox', ...
                                  'Value', p.val, ...
                                  'String', '', ...
                                  'UserData', p, ...
                                  'Callback', @(s,e)self.booleanParamCallback(s,e), ...
                                  'TooltipString',p.desc);
                    self.uicontrolH.ParameterUnitsText(ii) = ... % include a blank 'units string' here to keep the rest
                        uicontrol('Parent',self.uicontrolH.ParametersPanel, ...
                                  'Style','text', ...
                                  'HorizontalAlignment','left', ...
                                  'String',' ', ...
                                  'TooltipString',p.desc);
                else % Numeric edit field
                    self.uicontrolH.ParameterEdit(ii) = ...
                        uicontrol('Parent',self.uicontrolH.ParametersPanel, ...
                                  'Style','edit', ...
                                  'HorizontalAlignment','right', ...
                                  'String', p.val, ...
                                  'UserData', p, ...
                                  'Callback', @(s,e)self.numericParamCallback(s,e), ...
                                  'TooltipString',p.desc);
                    self.uicontrolH.ParameterUnitsText(ii) = ...
                        uicontrol('Parent',self.uicontrolH.ParametersPanel, ...
                                  'Style','text', ...
                                  'HorizontalAlignment','left', ...
                                  'String',p.units, ...
                                  'TooltipString',p.desc);
                end
            end

            fs = self.figureSpacing;

            panel_H = (numParams+1)*(fs.text_H+fs.buffer_H);
            panel_W = fs.panel_W;
            set(self.uicontrolH.ParametersPanel,'Position',[0 0 panel_W panel_H]);
            for ii=1:numParams
              row_Y = panel_H - 2*fs.buffer_H - (ii)*(fs.text_H+fs.buffer_H);
              set(self.uicontrolH.ParameterText(ii),'Position',[0 row_Y fs.name_W fs.text_H]);
              set(self.uicontrolH.ParameterEdit(ii),'Position',[fs.name_W+fs.buffer_W row_Y fs.edit_W fs.text_H]);
              set(self.uicontrolH.ParameterUnitsText(ii),'Position',[fs.name_W+fs.edit_W+fs.buffer_W*2 row_Y fs.units_W fs.text_H]);
            end
        end

        function booleanParamCallback(self, src, event)
            % TODO communicate with Teensy
            pName = src.UserData.name;
            self.user.(pName) = src.Value;
        end

        function numericParamCallback(self, src, event)
            pName = src.UserData.name;
            pMax = src.UserData.max;
            pMin = src.UserData.min;
            pStep = src.UserData.step;

            newVal = str2num(src.String);
            if ~isempty(newVal)
                newVal = pStep * round(newVal/pStep);
                newVal = max(pMin, newVal);
                newVal = min(pMax, newVal);

                % TODO set & get Teensy parameter
                self.setParam(pName, newVal)

                self.user.(pName) = newVal;
            end
            set(src, 'String', self.user.(pName))
        end

        function rewardButtonAction(self)
            self.Arduino.writeMessage('reward',0,0);
        end

        function multiRewardButtonAction(self)
            self.Arduino.writeMessage('multiReward',20,0);
        end

        function runSessionButtonAction(self)
            self.runSession();
        end

        function stopSessionButtonAction(self)
            self.stopSession();
        end

        function updateFigure(self)
            self.debugOut('Updating Figure!', 3);
            for ii=1:self.numParams
                uiEditControl = self.uicontrolH.ParameterEdit(ii);
                pName = uiEditControl.UserData.name;
                set(uiEditControl, 'String', self.user.(pName))
            end
            self.figurePosition = get(self.figureH, 'position');
        end

        function updateFigurePosition(self)
            self.debugOut('Updating Figure Pos.', 3);
            %currPos = get(self.figureH, 'position');
            if ~isempty(self.figurePosition)
                newPos = [self.figurePosition(1:2), self.figure_W, self.figure_H];
                % disp(newPos)
                set(self.figureH, 'position', newPos)
            end
            self.figurePosition = get(self.figureH, 'position');
        end

        function enableParameterEditing(self)
            for uic = self.uicontrolH.ParameterEdit
              set(uic,'Enable','on');
            end
        end

        function disableParameterEditing(self)
            for uic = self.uicontrolH.ParameterEdit
              set(uic,'Enable','off');
            end
        end

        function userMessage(self, message)
            msgList = self.uicontrolH.MessageList;
            msgList.String{end+1} = strtrim(message);
            set(msgList, 'Value', numel(msgList.String));
        end

    end % methods
    methods (Static)

        function fTimer = executeFunctionWithDelay(func, delay)
            % executeFunctionWithDelay: executes a function after a specified delay (in sec)

            fTimer = timer('ExecutionMode', 'singleShot', 'StartDelay', delay);
            fTimer.StopFcn = @(obj,event)delete(obj);
            fTimer.TimerFcn = {@(obj,event)func()};
            start(fTimer);
        end
    end % methods

    methods %(Static = true)
        function interpretArduinoMessage(self, messageString)
            fprintf('Arduino: %s\n',messageString);
            % return
            messageString = strtrim(messageString);

            % if message is a "comment",
            % print to log and return
            if (messageString(1)=='!')
                self.userMessage(messageString(2:end))
                return
            end

            parseArray = textscan(messageString, '%s %f %f');
            messageType = parseArray{1}{1};
            % fprintf('From Teensy: "%s" ',messageType)
            value = parseArray{2};
            if ~isempty(value)
                value = value(1);
            %     fprintf('%f ', value)
            end
            value2 = parseArray{3};
            if ~isempty(value2)
                value2 = value2(1);
            %     fprintf('%f ', value2)
            end
            % fprintf('\n')
            % disp(messageType)

            % check for special message types:
            if startsWith(messageType, 'S_') % Change of state
                state = messageType(3:end);
                % probably don't need to log every state change...
                % self.logEvent(state)

            elseif startsWith(messageType, 'P_') % Parameter value
                paramToChange = messageType(3:end);
                foundParam = false;
                % fprintf('Parameter from messge: %s\n', paramToChange)
                for pName = fieldnames(self.user)'
                    pName = pName{1};
                    % fprintf('       ?=  %s\n', pName)
                    if strcmp(paramToChange, pName)
                        foundParam = true;
                        self.user.(pName) = value;
                        self.updateFigure();
                        if self.isRunning
                            self.logValue(pName, value)
                        end
                        break
                    end
                end
                if ~foundParam
                    fprintf('Error: bad parameter sent from Arduino\n')
                end

            else
                switch messageType
                    % case 'numTrials'
                    %     % fprintf('numTrials: %d.\n', value)
                    %     self.user.numTrials = value;
                    %     self.updateFigure();
                    % case 'maxRunLength'
                    %     % fprintf('maxRunLength: %d.\n', value)
                    %     self.user.maxRunLength = value;
                    %     self.updateFigure();
                    % case 'pStim'
                    %     % fprintf('pStim: %d.\n', value)
                    %     self.user.pStim = value;
                    %     self.updateFigure();
                    % case 'numRewardedOdors'
                    %     % fprintf('numRewardedOdors : %d.\n', value)
                    %     self.user.numRewardedOdors  = value;
                    %     self.updateFigure();
                    % case 'stimulusDuration_ms'
                    %     % fprintf('stimulusDuration_ms: %d.\n', value)
                    %     self.user.stimulusDuration_ms = value;
                    %     self.updateFigure();
                    % case 'numLickBins'
                    %     % fprintf('numLickBins: %d.\n', value)
                    %     self.user.numLickBins = value;
                    %     self.updateFigure();
                    % case 'minLickBins'
                    %     % fprintf('minLickBins: %d.\n', value)
                    %     self.user.minLickBins = value;
                    %     self.updateFigure();
                    % case 'rewardDuration_ms'
                    %     % fprintf('rewardDuration_ms: %d.\n', value)
                    %     self.user.rewardDuration_ms = value;
                    %     self.updateFigure();
                    % case 'interTrialDuration_ms'
                    %     % fprintf('interTrialDuration_ms: %d.\n', value)
                    %     self.user.interTrialDuration_ms = value;
                    %     self.updateFigure();

                    % case 'rewardedOdorPin'
                    %     fprintf('rewardedOdorPin: %d.\n', value)
                    %     self.user.rewardedOdorPin = value;
                    %     self.updateFigure();
                    % case 'blankOdorPin'
                    %     fprintf('blankOdorPin: %d.\n', value)
                    %     self.user.blankOdorPin = value;
                    %     self.updateFigure();
                    % case 'unrewardedOdorPin'
                    %     fprintf('unrewardedOdorPin: %d.\n', value)
                    %     self.user.unrewardedOdorPin = value;
                    %     self.updateFigure();
                    % case 'beambreakPin'
                    %     fprintf('beambreakPin: %d.\n', value)
                    %     self.user.beambreakPin = value;
                    %     self.updateFigure();
                    % case 'lickPin'
                    %     fprintf('lickPin: %d.\n', value)
                    %     self.user.lickPin = value;
                    %     self.updateFigure();
                    % case 'LEDPin'
                    %     fprintf('LEDPin: %d.\n', value)
                    %     self.user.LEDPin = value;
                    %     self.updateFigure();
                    % case 'rewardPin'
                    %     fprintf('rewardPin: %d.\n', value)
                    %     self.user.rewardPin = value;
                    %     self.updateFigure();


                    case 'T'
                        self.logValue('Trial', value)
                        self.trialIndex = value;
                        if self.n_trials_since_switch >= self.user.numLookBack && ...
                                self.getAccuracy(self.user.numLookBack) >= self.user.accThresh
                                %self.debugOut(sprintf('At delay %0.3f, accuracy of %0.3f over the past %d trials was above threshold of %0.3f', ...
                                %    self.current_pav_delay, self.getAccuracy(self.user.numLookBack), self.n_trials_since_switch, self.user.accuracyThresh),1);
                                %self.updatePavDelay();
                                self.updateDelay();
%                                     self.stimDurationCounter = self.stimDurationCounter + 1;
%                                     newStimDuration = self.stimDurations(self.stimDurationCounter);                                  
                                self.n_trials_since_switch = 0;
                        else
                            self.n_trials_since_switch = self.n_trials_since_switch + 1;
                        end
                    case 'SS'
                        self.logValue('Stimulus_On', value)
                        if value > 0
                            self.odorTypeList(self.trialIndex) = 1;
                        end
                    case 'SE'
                        self.logEvent('Stimulus_Off')
                    case 'RS' % Reward Start
                        self.logEvent('Reward_Start')
                    case 'RE' % Reward End
                        self.logEvent('Reward_End')
                    case 'L'
                        self.logEvent('Lick')
                    case 'LI'
                        self.logEvent('Lick_On')
                    case 'LO'
                        self.logEvent('Lick_Off')
                    case 'NI'
                        self.logEvent('Nose_In')
                    case 'NO'
                        self.logEvent('Nose_Out')

                    case 'OH'
                        self.logEvent('TRIAL_END_HIT')
                        self.accuracy(self.trialIndex) = 1;
                    case 'OM'
                        self.logEvent('TRIAL_END_MISS')
                    case 'OCR'
                        self.logEvent('TRIAL_END_CORRECT_REJECTION')
                    case 'OFA'
                        self.logEvent('TRIAL_END_FALSE_ALARM')


                    case '$'
                        % Teensy started up; do nothing
                        % fprintf('$\n');
                    case 'E'
                        % Session has [E]nded
                        self.sessionHasStopped();
                    case 'A'
                        % Session was [A]borted
                        self.logEvent('Aborted_by_user');
                        self.sessionHasStopped();


                    case '#'
                        fprintf('### Arduino ERROR Code %d:\n        ', value);
                        switch value
                            case 1
                                fprintf('Warning: Valve 1 cannot be directly closed in ''ONE_VALVE_OPEN'' mode.\n');
                            case 2
                                fprintf('Valve %d is already open.\n',value2);
                        end
                        fprintf('\n');
                    otherwise
                        % unknown input
                        %fprintf('Received unknown message type: %s\n',messageType)
                        fprintf('?? %s\n',messageString)
                end
            end
        end
    end % static Arduino methods

end  % classdef
