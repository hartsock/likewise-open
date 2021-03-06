/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

using System.Drawing;
using System.Windows.Forms;
using System;
using System.Xml;
using System.Collections.Generic;
using Likewise.LMC.Plugins.FileShareManager.Properties;
using Likewise.LMC.ServerControl;
using Likewise.LMC.Utilities;
using Likewise.LMC.NETAPI;
using Likewise.LMC.Plugins.FileShareManager;

namespace Likewise.LMC.Plugins.FileShareManager
{
    public class FileShareManagerIPlugIn : IPlugIn
    {
        #region Enum variables

        public enum PluginNodeType
        {
            SHARES,
            SESSIONS,
            OPENFILES,
            PRINTERS,
            UNDEFINED
        }
        #endregion

        #region Class data

        private string _currentHost = "";
        private IPlugInContainer _container;
        private Hostinfo _hn;
        private LACTreeNode _pluginNode;
        public FileHandle fileHandle = null;

        private List<IPlugIn> _extPlugins = null;

        #endregion

        #region IPlugIn Members

        public Hostinfo HostInfo
        {
            get
            {
                return _hn;
            }
        }

        public string GetName()
        {
            Logger.Log("FileShareManagerPlugIn.GetName", Logger.FileShareManagerLogLevel);

            return Resources.sTitleTopLevelTab;
        }

        public string GetDescription()
        {
            return Resources.PluginDescription;
        }

        public string GetPluginDllName()
        {
            return "Likewise.LMC.Plugins.FileShareManager.dll";
        }

        public IContextType GetContextType()
        {
            return IContextType.Hostinfo;
        }

        public void SerializePluginInfo(LACTreeNode pluginNode, ref int Id, out XmlElement viewElement, XmlElement ViewsNode, TreeNode SelectedNode)
        {
            viewElement = null;

            try
            {
                if (pluginNode == null || !pluginNode._IsPlugIn)
                    return;

                XmlElement HostInfoElement = null;
                XmlElement gpoInfoElement = null;
                GPObjectInfo gpoInfo = pluginNode.Tag as GPObjectInfo;

                Manage.InitSerializePluginInfo(pluginNode, this, ref Id, out viewElement, ViewsNode, SelectedNode);

                Manage.CreateAppendHostInfoElement(_hn, ref viewElement, out HostInfoElement);
                Manage.CreateAppendGPOInfoElement(gpoInfo, ref HostInfoElement, out gpoInfoElement);

                if (pluginNode != null && pluginNode.Nodes.Count != 0)
                {
                    foreach (LACTreeNode lacnode in pluginNode.Nodes)
                    {
                        XmlElement innerelement = null;
                        pluginNode.Plugin.SerializePluginInfo(lacnode, ref Id, out innerelement, viewElement, SelectedNode);
                        if (innerelement != null)
                        {
                            viewElement.AppendChild(innerelement);
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                Logger.LogException("FileShareManagerPlugin.SerializePluginInfo()", ex);
            }
        }

        public void DeserializePluginInfo(XmlNode node, ref LACTreeNode pluginNode, string nodepath)
        {
            try
            {
                Manage.DeserializeHostInfo(node, ref pluginNode, nodepath, ref _hn, false);
                pluginNode.Text = this.GetName();
                pluginNode.Name = this.GetName();
            }
            catch (Exception ex)
            {
                Logger.LogException("FileShareManagerPlugin.DeserializePluginInfo()", ex);
            }
        }

        public void AddExtPlugin(IPlugIn extPlugin)
        {
            if (_extPlugins == null)
            {
                _extPlugins = new List<IPlugIn>();
            }

            _extPlugins.Add(extPlugin);
        }

        public void Initialize(IPlugInContainer container)
        {
            Logger.Log("FileShareManagerPlugIn.Initialize", Logger.FileShareManagerLogLevel);

            _container = container;
        }

        public void SetContext(IContext ctx)
        {
            Hostinfo hn = ctx as Hostinfo;

            Logger.Log(String.Format("FileShareManagerPlugIn.SetHost(hn: {0}\n)",
            hn == null ? "<null>" : hn.ToString()), Logger.eventLogLogLevel);

            bool deadTree = false;

            if (_pluginNode != null &&
                _pluginNode.Nodes != null &&
                _hn != null &&
                hn != null &&
                hn.hostName !=
                _hn.hostName)
            {
                foreach (TreeNode node in _pluginNode.Nodes)
                {
                    _pluginNode.Nodes.Remove(node);
                }
                deadTree = true;
            }

            _hn = hn;

            if (HostInfo == null)
            {
                _hn = new Hostinfo();
            }

            ConnectToHost();

            if (_pluginNode != null && _pluginNode.Nodes.Count == 0 && _hn.IsConnectionSuccess)
            {
                BuildNodesToPlugin();
            }

            if (deadTree && _pluginNode != null)
            {
                _pluginNode.SetContext(_hn);
            }
        }

        public IContext GetContext()
        {
            return _hn;
        }

        public LACTreeNode GetPlugInNode()
        {
            return GetFileShareManagerNode();
        }

        public void EnumChildren(LACTreeNode parentNode)
        {
            Logger.Log("FileShareManagerPlugIn.EnumChildren", Logger.FileShareManagerLogLevel);

            return;
        }

        public void SetCursor(System.Windows.Forms.Cursor cursor)
        {
            Logger.Log("FileShareManagerPlugIn.SetCursor", Logger.FileShareManagerLogLevel);

            if (_container != null)
            {
                _container.SetCursor(cursor);
            }
        }

        public ContextMenu GetTreeContextMenu(LACTreeNode nodeClicked)
        {
            Logger.Log("FileShareManagerPlugIn.GetTreeContextMenu", Logger.FileShareManagerLogLevel);

            if (nodeClicked == null)
            {
                return null;
            }
            else
            {
                ContextMenu fileShareManagerContextMenu = null;

                StandardPage fileShareManagerPage = (StandardPage)nodeClicked.PluginPage;

                if (fileShareManagerPage == null)
                {
                    Type type = nodeClicked.NodeType;
                    object o = Activator.CreateInstance(type);
                    if (o is IPlugInPage)
                    {
                        ((IPlugInPage)o).SetPlugInInfo(_container, nodeClicked.Plugin, nodeClicked, (LWTreeView)nodeClicked.TreeView, nodeClicked.sc);
                    }
                }
                if (_pluginNode == nodeClicked)
                {
                    fileShareManagerContextMenu = new ContextMenu();

                    MenuItem m_item = new MenuItem("Set Target Machine", new EventHandler(cm_OnConnect));
                    fileShareManagerContextMenu.MenuItems.Add(0, m_item);
                }
                else if (nodeClicked.Name.Trim().Equals(Resources.FileShares))
                {
                    SharesPage sharesPage = fileShareManagerPage as SharesPage;
                    if (sharesPage != null)
                    {
                        fileShareManagerContextMenu = sharesPage.GetTreeContextMenu();
                    }
                }
                else if (nodeClicked.Name.Trim().Equals(Resources.OpenSessions))
                {
                    SessionPage sessionPage = fileShareManagerPage as SessionPage;
                    if (sessionPage != null)
                    {
                        fileShareManagerContextMenu = sessionPage.GetTreeContextMenu();
                    }
                }
                else if (nodeClicked.Name.Trim().Equals(Resources.OpenFiles))
                {
                    FilesPage filesPage = fileShareManagerPage as FilesPage;
                    if (filesPage != null)
                    {
                        fileShareManagerContextMenu = filesPage.GetTreeContextMenu();
                    }
                }
                return fileShareManagerContextMenu;
            }
        }

        public void SetSingleSignOn(bool useSingleSignOn)
        {
            // do nothing
        }

        public bool PluginSelected()
        {
            return true;
        }

        #endregion

        #region Private helper functions

        private LACTreeNode GetFileShareManagerNode()
        {
            Logger.Log("FileShareManagerPlugIn.GetFileShareManagerNode", Logger.FileShareManagerLogLevel);

            if (_pluginNode == null)
            {
                Icon ic = Resources.SharedFolder2;
                _pluginNode = Manage.CreateIconNode("File Browser", ic, typeof(FilesBrowserPluginPage), this);
                _pluginNode.ImageIndex = (int)Manage.ManageImageType.Generic;
                _pluginNode.SelectedImageIndex = (int)Manage.ManageImageType.Generic;

                _pluginNode.IsPluginNode = true;
            }

            return _pluginNode;
        }

        private void BuildNodesToPlugin()
        {
            if (_pluginNode != null)
            {
                Icon ic = Resources.SharedFolder2;
                LACTreeNode shNode = Manage.CreateIconNode(Resources.FileShares, ic, typeof(FileSharesPage), this);
                _pluginNode.Nodes.Add(shNode);

                LACTreeNode osNode = Manage.CreateIconNode(Resources.OpenSessions, ic, typeof(SessionPage), this);
                _pluginNode.Nodes.Add(osNode);

                LACTreeNode ofNode = Manage.CreateIconNode(Resources.sOpenFiles, ic, typeof(FilesPage), this);
                _pluginNode.Nodes.Add(ofNode);
            }
        }

        private void ConnectToHost()
        {
            Logger.Log("FileShareManagerPlugIn.ConnectToHost", Logger.FileShareManagerLogLevel);

            if (_hn.creds.Invalidated)
            {
                _container.ShowError("File Browser PlugIn cannot connect to domain due to invalid credentials");
                _hn.IsConnectionSuccess = false;
                return;
            }
            if (!String.IsNullOrEmpty(_hn.hostName))
            {
                if (_currentHost != _hn.hostName)
                {
                    if (fileHandle != null)
                    {
                        fileHandle.Dispose();
                        fileHandle = null;
                    }

                    if (_pluginNode != null && !String.IsNullOrEmpty(_hn.hostName))
                    {
                        OpenHandle(_hn.hostName);
                    }

                    _currentHost = _hn.hostName;
                }
                _hn.IsConnectionSuccess = true;
            }
            else
                _hn.IsConnectionSuccess = false;
        }

        private void cm_OnConnect(object sender, EventArgs e)
        {
            //check if we are joined to a domain -- if not, use simple bind
            uint requestedFields = (uint)Hostinfo.FieldBitmaskBits.FQ_HOSTNAME;
            //string domainFQDN = null;

            if (_hn == null)
            {
                _hn = new Hostinfo();
            }

            //TODO: kerberize eventlog, so that creds are meaningful.
            //for now, there's no reason to attempt single sign-on
            requestedFields |= (uint)Hostinfo.FieldBitmaskBits.FORCE_USER_PROMPT;

            if (_hn != null)
            {
                if (!_container.GetTargetMachineInfo(this, _hn, requestedFields))
                {
                    Logger.Log(
                    "Could not find information about target machine",
                    Logger.FileShareManagerLogLevel);
                }
                else
                {
                    if (_pluginNode != null && !String.IsNullOrEmpty(_hn.hostName))
                    {
                        _pluginNode.sc.ShowControl(_pluginNode);
                    }
                    else
                    {
                        Logger.ShowUserError("Unable to find the hostname that enterted");
                        _hn.IsConnectionSuccess = false;
                    }
                }
            }
        }

        #endregion

        #region eventlog API wrappers

        public void OpenHandle(string hostname)
        {
            try
            {
                if (fileHandle == null)
                {
                    fileHandle = HandleAdapter.OpenHandle(hostname);
                }
            }
            catch (Exception e)
            {
                Logger.LogException("EventViewerPlugin.OpenEventLog", e);
                fileHandle = null;
            }
        }

        public bool IsHandleBinded()
        {
            if (fileHandle == null)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        public void CloseEventLog()
        {
            if (fileHandle == null)
            {
                return;
            }
            fileHandle.Dispose();
            fileHandle = null;
        }

        #endregion
    }
}
