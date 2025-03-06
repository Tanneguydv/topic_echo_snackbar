/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <iostream>
#include <QRegularExpression>
#include <gz/common/Console.hh>
#include <gz/plugin/Register.hh>
#include <gz/transport/Node.hh>

#include "gz/gui/Application.hh"
#include "gz/gui/MainWindow.hh" 

#include "TopicEchoSnackBar.hh"


namespace ignition
{
namespace gui
{
namespace plugins
{
  class TopicEchoSnackBarPrivate
  {
    /// \brief Topic
    public: QString topic{"/echo_snackbar"};

    /// \brief A list of text data.
    public: QStringListModel msgList;

    /// \brief Size of the text buffer. The size is the number of
    /// messages.
    public: unsigned int buffer{10u};

    /// \brief Flag used to pause message parsing.
    public: bool paused{false};

    /// \brief Mutex to protect message buffer.
    public: std::mutex mutex;

    /// \brief Node for communication
    public: gz::transport::Node node;
  };
}
}
}

// using namespace gz;
// using namespace gui;
// using namespace plugins;

// Replace with explicit namespace qualification:
using ignition::gui::Plugin;  // <<< Explicitly reference ignition's Plugin

/////////////////////////////////////////////////
ignition::gui::plugins::TopicEchoSnackBar::TopicEchoSnackBar()
  : Plugin(), dataPtr(new TopicEchoSnackBarPrivate)
{
  // Connect model
  App()->Engine()->rootContext()->setContextProperty("TopicEchoSnackBarMsgList",
      &this->dataPtr->msgList);
}

/////////////////////////////////////////////////
ignition::gui::plugins::TopicEchoSnackBar::~TopicEchoSnackBar() = default;
// {
// }

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::LoadConfig(const tinyxml2::XMLElement * /*_pluginElem*/)
{
  if (this->title.empty())
    this->title = "Topic Echo SnackBar";

  // this->connect(this, SIGNAL(AddMsg(QString)), this, SLOT(OnAddMsg(QString)),
  //         Qt::QueuedConnection);
  connect(this, &TopicEchoSnackBar::AddMsg, this, &TopicEchoSnackBar::OnAddMsg, Qt::QueuedConnection);

}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::Stop()
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  // Erase all previous messages
  this->dataPtr->msgList.removeRows(0,
      this->dataPtr->msgList.rowCount());

  // Unsubscribe
  for (auto const &sub : this->dataPtr->node.SubscribedTopics())
    this->dataPtr->node.Unsubscribe(sub);
}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::OnEcho(const bool _checked)
{
  this->Stop();

  if (!_checked)
    return;

  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  // Subscribe to new topic
  auto topic = this->dataPtr->topic.toStdString();
  if (!this->dataPtr->node.Subscribe(topic, &TopicEchoSnackBar::OnMessage, this))
  {
    ignerr << "Invalid topic [" << topic << "]" << std::endl;
  }
}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::OnMessage(const google::protobuf::Message &_msg)
{
  if (this->dataPtr->paused)
    return;

  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  // Debug: Print the message type
  ignmsg << "Received message of type: " << _msg.GetTypeName() << std::endl;

  // Check if the message is of the expected type
  const ignition::msgs::StringMsg* stringMsg = dynamic_cast<const ignition::msgs::StringMsg*>(&_msg);
  if (!stringMsg)
  {
    ignerr << "Received message is not of type ignition.msgs.StringMsg" << std::endl;
    return;
  }

  // Debug: Print the message data
  ignmsg << "Message data: " << stringMsg->data() << std::endl;

  // Add the message data to the list
  this->AddMsg(QString::fromStdString(stringMsg->data()));
}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::OnAddMsg(QString _msg)
{
  ignmsg << "Received message: " << _msg.toStdString() << std::endl;

  if (!App() || !App()->Engine()) {
    ignerr << "App or Engine is not valid" << std::endl;
    return;
  }

  MainWindow *mainWindow = App()->findChild<MainWindow *>();
  if (!mainWindow) {
    ignerr << "MainWindow is not valid. Ensure the GUI is fully initialized." << std::endl;
    return;
  }

  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);

  QRegularExpression boldTagRegex("\\/\\/(.*?)\\/\\/");
  QRegularExpression colorTagRegex("^<(\\w+)>(.*)");
  QRegularExpression durationRegex("--duration:=([0-9]+)");

  QString formattedMsg = _msg;
  int duration = 4000;

  QRegularExpressionMatch match = durationRegex.match(formattedMsg);
  if (match.hasMatch()) {
    duration = match.captured(1).toInt();
    formattedMsg.remove(durationRegex);
  }

  match = colorTagRegex.match(formattedMsg);
  if (match.hasMatch()) {
    QString color = match.captured(1);
    formattedMsg = QString("<span style='color:%1;'>%2</span>").arg(color, match.captured(2));
  }

  formattedMsg.replace(boldTagRegex, "<b>\\1</b>");
  ignmsg << "Formatted message: " << formattedMsg.toStdString() << std::endl;

  if (this->dataPtr->msgList.insertRow(this->dataPtr->msgList.rowCount())) {
    auto index = this->dataPtr->msgList.index(this->dataPtr->msgList.rowCount() - 1, 0);
    this->dataPtr->msgList.setData(index, formattedMsg);
  }

  mainWindow->notifyWithDuration(QString("%1").arg(formattedMsg), duration);

  auto diff = this->dataPtr->msgList.rowCount() - this->dataPtr->buffer;
  this->dataPtr->msgList.removeRows(0, diff);
}

/////////////////////////////////////////////////
QString ignition::gui::plugins::TopicEchoSnackBar::Topic() const
{
  return this->dataPtr->topic;
}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::SetTopic(const QString &_topic)
{
  this->dataPtr->topic = _topic;
  // Notify the GUI
  // emit App()->findChild<ignition::gui::MainWindow *>()->notifyWithDuration(
  this->TopicChanged();
}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::OnBuffer(const unsigned int _buffer)
{
  this->dataPtr->buffer = _buffer;
}

/////////////////////////////////////////////////
bool ignition::gui::plugins::TopicEchoSnackBar::Paused() const
{
  std::lock_guard<std::mutex> lock(this->dataPtr->mutex);
  return this->dataPtr->paused;
}

/////////////////////////////////////////////////
void ignition::gui::plugins::TopicEchoSnackBar::SetPaused(const bool &_paused)
{
  this->dataPtr->paused = _paused;
  this->PausedChanged();
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::plugins::TopicEchoSnackBar,
  ignition::gui::Plugin)
