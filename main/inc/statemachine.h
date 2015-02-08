/*
 *  Copyright (C) 2014 Marcel Lehwald
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#define LOGGER_STATEMACHINE "statemachine"

#include <logger.h>
#include <value.h>

#include <QEvent>
#include <QAbstractTransition>
#include <QFinalState>
#include <QState>
#include <QStateMachine>

namespace hfsmexec
{
    class AbstractState;
    class StateMachine;
    class CommunicationPlugin;

    class AbstractEvent : public QEvent
    {
        public:
            AbstractEvent(Type type);

            virtual QString toString() const = 0;

        protected:
            static const Logger* logger;
    };

    class AbstractTransition : public QAbstractTransition
    {
        friend class StateMachineBuilder;

        public:
            AbstractTransition(const QString transitionId, const QString sourceStateId, const QString targetStateId);
            virtual ~AbstractTransition();

            const QString& getId() const;
            AbstractState* getSourceState();
            AbstractState* getTargetState();
            StateMachine* getStateMachine();

            bool initialize();

            virtual QString toString() const = 0;

        protected:
            static const Logger* logger;
            QString transitionId;
            QString sourceStateId;
            QString targetStateId;
            AbstractState* sourceState;
            AbstractState* targetState;
            StateMachine* stateMachine;

            virtual bool eventTest(QEvent* e) = 0;
            virtual void onTransition(QEvent* e) = 0;
    };

    class Dataflow
    {
        friend class StateMachineBuilder;

        public:
            Dataflow(const QString& sourceStateId, const QString& targetStateId, const QString& from, const QString& to);
            ~Dataflow();

            const QString& getSourceStateId() const;
            const QString& getTargetStateId() const;
            const QString& getFrom() const;
            const QString& getTo() const;

            AbstractState* getSourceState();
            AbstractState* getTargetState();
            Value& getFromParameter();
            Value& getToParameter();

            QString toString() const;

        private:
            QString sourceStateId;
            QString targetStateId;
            QString from;
            QString to;
            AbstractState* sourceState;
            AbstractState* targetState;
            Value fromParameter;
            Value toParameter;
    };

    class AbstractState : public QObject
    {
        Q_OBJECT

        friend class StateMachineBuilder;

        public:
            AbstractState(const QString& stateId, const QString& parentStateId = "");
            virtual ~AbstractState();

            const QString& getId() const;
            void setId(const QString& stateId);
            const QString& getParentStateId() const;
            void setParentStateId(const QString& parentStateId);
            const StateMachine* getStateMachine() const;

            Value& getInputParameters();
            void setInputParameters(const Value& value);
            Value& getOutputParameters();
            void setOutputParameters(const Value& value);

            const QList<Dataflow*>& getDataflows() const;

            AbstractState* getParentState();

            const QList<AbstractState*>& getChildStates() const;
            AbstractState* getChildState(const QString& stateId);

            const QList<AbstractTransition*>& getTransitions() const;
            AbstractTransition* getTransition(const QString& transitionId);

            AbstractState* findState(const QString& stateId);

            virtual QAbstractState* getDelegate() const = 0;
            virtual bool initialize() = 0;
            virtual QString toString() const = 0;

        protected:
            static const Logger* logger;
            QString stateId;
            QString parentStateId;
            StateMachine* stateMachine;
            Value inputParameters;
            Value outputParameters;
            QList<Dataflow*> dataflows;
            QList<AbstractTransition*> transitions;
            QList<AbstractState*> childStates;
    };

    class AbstractComplexState : public AbstractState
    {
        Q_OBJECT

        public:
            AbstractComplexState(const QString& stateId, const QString& parentStateId = "");
            virtual ~AbstractComplexState();

            virtual QState* getDelegate() const;
            virtual bool initialize() = 0;
            virtual QString toString() const = 0;

        protected slots:
            virtual void eventEntered();
            virtual void eventExited();
            virtual void eventFinished();

        protected:
            QState* delegate;
    };

    class NamedEvent : public AbstractEvent
    {
        public:
            static const QEvent::Type type;

            NamedEvent(const QString& eventName);
            ~NamedEvent();

            const QString& getEventName() const;
            void setEventName(const QString& eventName);

            const QString& getOrigin() const;
            void setOrigin(const QString& origin);

            const QString& getMessage() const;
            void setMessage(const QString& message);

            virtual QString toString() const;

        private:
            QString eventName;
            QString origin;
            QString message;
    };

    class NamedTransition : public AbstractTransition
    {
        public:
            NamedTransition(const QString& transitionId, const QString& sourceStateId, const QString& targetStateId, const QString& eventName);

            virtual QString toString() const;

        protected:
            virtual bool eventTest(QEvent* e);
            virtual void onTransition(QEvent* e);

        private:
            QString eventName;
    };

    class InternalEvent : public QEvent
    {
        public:
            static const QEvent::Type type;

            InternalEvent(const QString& eventName);

            const QString& getEventName() const;

        private:
            QString eventName;
    };

    class InternalTransition : public QAbstractTransition
    {
        public:
            InternalTransition(const QString& eventName);

        protected:
            virtual bool eventTest(QEvent* e);
            virtual void onTransition(QEvent* e);

        private:
            QString eventName;
    };

    class FinalState : public AbstractState
    {
        Q_OBJECT

        public:
            FinalState(const QString& stateId, const QString& parentStateId = "");
            ~FinalState();

            virtual QFinalState* getDelegate() const;
            virtual bool initialize();
            virtual QString toString() const;

        private:
            QFinalState* delegate;
    };


    class CompositeState : public AbstractComplexState
    {
        Q_OBJECT

        public:
            CompositeState(const QString& stateId, const QString &initialStateId, const QString& parentStateId = "");
            ~CompositeState();

            virtual bool initialize();
            virtual QString toString() const;

        private:
            QString initialStateId;
    };

    class ParallelState : public AbstractComplexState
    {
        Q_OBJECT

        public:
            ParallelState(const QString& stateId, const QString& parentStateId = "");
            ~ParallelState();

            virtual bool initialize();
            virtual QString toString() const;
    };

    class InvokeState : public AbstractComplexState
    {
        Q_OBJECT

        public:
            InvokeState(const QString& stateId, const QString& type, const QString& parentStateId = "");
            virtual ~InvokeState();

            Value& getEndpoint();
            void setEndpoint(Value& value);

            CommunicationPlugin* getCommunicationPlugin();
            void setCommunicationPlugin(CommunicationPlugin* value);

            void done();

            virtual bool initialize();
            virtual QString toString() const;

        protected slots:
            virtual void eventEntered();
            virtual void eventExited();
            virtual void eventFinished();

        private:
            QString type;
            CommunicationPlugin* communicationPlugin;
            Value endpoint;
    };

    class StateMachine : public AbstractComplexState
    {
        Q_OBJECT

        friend class StateMachineBuilder;

        public:
            StateMachine(const QString& stateId, const QString& initialId, const QString& parentStateId = "");
            ~StateMachine();

            void start() const;
            void stop() const;

            int postDelayedEvent(QEvent* event, int delay);
            void postEvent(QEvent* event, QStateMachine::EventPriority priority = QStateMachine::NormalPriority);

            virtual QStateMachine* getDelegate() const;
            virtual bool initialize();
            virtual QString toString() const;

        protected slots:
            virtual void eventStarted();
            virtual void eventStopped();

        protected:
            QStateMachine* delegate;

        private:
            QString initialId;
    };
}

#endif
