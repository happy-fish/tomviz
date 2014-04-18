/******************************************************************************

  This source file is part of the TEM tomography project.

  Copyright Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/
#ifndef __Behaviors_h
#define __Behaviors_h

#include <QObject>

class QMainWindow;
namespace TEM
{
/// Behaviors instantiates MatViz relevant ParaView behaviors (and any new
/// ones) as needed.
class Behaviors : public QObject
  {
  Q_OBJECT

  typedef QObject Superclass;

public:
  Behaviors(QMainWindow* mainWindow);
  virtual ~Behaviors();

private:
  Q_DISABLE_COPY(Behaviors)
};
}
#endif
