﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using LWDicer.Control;

namespace LWDicer.UI
{
    public partial class FormOperatorMaint : Form
    {
        private PageInfo PrevPage = null;
        private PageInfo NextPage = null;
        private BottomPageInfo NextBottomPage = null;

        public void SetPrevPage(PageInfo page)
        {
            PrevPage = page;
        }

        public void SetNextPage(PageInfo page)
        {
            NextPage = page;
        }

        public FormOperatorMaint()
        {
            InitializeComponent();
        }
        public void SetNextBottomPage(BottomPageInfo page)
        {
            NextBottomPage = page;
        }
        protected virtual void InitializeForm()
        {
            this.StartPosition = System.Windows.Forms.FormStartPosition.Manual;
            this.DesktopLocation = new Point(DEF_UI.MAIN_POS_X, DEF_UI.MAIN_POS_Y);
            this.Size = new Size(DEF_UI.MAIN_SIZE_WIDTH, DEF_UI.MAIN_SIZE_HEIGHT);
            this.FormBorderStyle = FormBorderStyle.None;
        }

        private void BtnNext_Click(object sender, EventArgs e)
        {
            if (NextPage == null)
            {
                return;
            }

            CMainFrame.MainFrame.MoveToPage(NextPage);
        }

        private void BtnExit_Click(object sender, EventArgs e)
        {
            if (PrevPage == null)
            {
                return;
            }

            CMainFrame.MainFrame.MoveToPage(PrevPage);
            CMainFrame.MainFrame.MoveToBottomPage(NextBottomPage);
        }
    }
}
