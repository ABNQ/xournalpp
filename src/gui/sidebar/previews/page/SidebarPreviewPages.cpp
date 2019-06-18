#include "SidebarPreviewPages.h"

#include "control/Control.h"
#include "control/PdfCache.h"
#include "gui/sidebar/previews/base/SidebarToolbar.h"
#include "SidebarPreviewPageEntry.h"
#include "undo/CopyUndoAction.h"
#include "undo/SwapUndoAction.h"

#include "i18n.h"
#include "util/cpp14memory.h"

SidebarPreviewPages::SidebarPreviewPages(Control* control, GladeGui* gui, SidebarToolbar* toolbar):
	SidebarPreviewBase(control, gui, toolbar), contextMenu(gui->get("sidebarPreviewContextMenu")) {
	XOJ_INIT_TYPE(SidebarPreviewPages);

	// Connect the context menu actions
	const std::map<std::string, SidebarActions> ctxMenuActions = {
		{"sidebarPreviewDuplicate", SIDEBAR_ACTION_COPY},
		{"sidebarPreviewDelete", SIDEBAR_ACTION_DELETE},
		{"sidebarPreviewMoveUp", SIDEBAR_ACTION_MOVE_UP},
		{"sidebarPreviewMoveDown", SIDEBAR_ACTION_MOVE_DOWN},
		{"sidebarPreviewNewBefore", SIDEBAR_ACTION_NEW_BEFORE},
		{"sidebarPreviewNewAfter", SIDEBAR_ACTION_NEW_AFTER},
	};

	for (auto& pair : ctxMenuActions) {
		GtkWidget* entry = gui->get(pair.first);
		g_assert(entry != nullptr);

		// Unfortunately, we need a fairly complicated mechanism to keep track
		// of which action we want to execute.
		typedef SidebarPreviewPages::ContextMenuData Data;
		auto userdata = std::unique_ptr<Data>(new Data { this->toolbar, pair.second });

		auto callback = G_CALLBACK(
			+[](GtkMenuItem* item, Data* data) {
				data->toolbar->runAction(data->actions);
			});
		gulong signalId = g_signal_connect(entry, "activate", callback, userdata.get());
		g_object_ref(entry);
		this->contextMenuSignals.push_back(std::make_tuple(entry, signalId, std::move(userdata)));
	}
}

SidebarPreviewPages::~SidebarPreviewPages()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	for (auto& signalTuple : this->contextMenuSignals) {
		GtkWidget* widget = std::get<0>(signalTuple);
		guint handlerId = std::get<1>(signalTuple);
		if (g_signal_handler_is_connected(widget, handlerId)) {
			g_signal_handler_disconnect(widget, handlerId);
		}
		g_object_unref(widget);
	}

	XOJ_RELEASE_TYPE(SidebarPreviewPages);
}

string SidebarPreviewPages::getName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return _("Page Preview");
}

string SidebarPreviewPages::getIconName()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	return "sidebar-page-preview";
}

/**
 * Called when an action is performed
 */
void SidebarPreviewPages::actionPerformed(SidebarActions action)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	switch (action)
	{
	case SIDEBAR_ACTION_MOVE_UP:
	{
		Document* doc = control->getDocument();
		PageRef swappedPage = control->getCurrentPage();
		if (!swappedPage.isValid())
		{
			return;
		}

		doc->lock();
		size_t page = doc->indexOf(swappedPage);
		PageRef otherPage = doc->getPage(page - 1);
		if (page != size_t_npos)
		{
			doc->deletePage(page);
			doc->insertPage(swappedPage, page - 1);
		}
		doc->unlock();

		UndoRedoHandler* undo = control->getUndoRedoHandler();
		undo->addUndoAction(mem::make_unique<SwapUndoAction>(page - 1, true, swappedPage, otherPage));

		control->firePageDeleted(page);
		control->firePageInserted(page - 1);
		control->firePageSelected(page - 1);

		control->getScrollHandler()->scrollToPage(page - 1);
		break;
	}
	case SIDEBAR_ACTION_MOVE_DOWN:
	{
		Document* doc = control->getDocument();
		PageRef swappedPage = control->getCurrentPage();
		if (!swappedPage.isValid())
		{
			return;
		}

		doc->lock();
		size_t page = doc->indexOf(swappedPage);
		PageRef otherPage = doc->getPage(page + 1);
		if (page != size_t_npos)
		{
			doc->deletePage(page);
			doc->insertPage(swappedPage, page + 1);
		}
		doc->unlock();

		UndoRedoHandler* undo = control->getUndoRedoHandler();
		undo->addUndoAction(mem::make_unique<SwapUndoAction>(page, false, swappedPage, otherPage));

		control->firePageDeleted(page);
		control->firePageInserted(page + 1);
		control->firePageSelected(page + 1);

		control->getScrollHandler()->scrollToPage(page + 1);
		break;
	}
	case SIDEBAR_ACTION_COPY:
	{
		Document* doc = control->getDocument();
		PageRef currentPage = control->getCurrentPage();
		if (!currentPage.isValid())
		{
			return;
		}

		doc->lock();
		size_t page = doc->indexOf(currentPage);

		PageRef newPage = currentPage.clone();
		doc->insertPage(newPage, page + 1);

		doc->unlock();

		UndoRedoHandler* undo = control->getUndoRedoHandler();
		undo->addUndoAction(mem::make_unique<CopyUndoAction>(newPage, page + 1));

		control->firePageInserted(page + 1);
		control->firePageSelected(page + 1);

		control->getScrollHandler()->scrollToPage(page + 1);
		break;
	}
	case SIDEBAR_ACTION_DELETE:
		control->deletePage();
		break;
	case SIDEBAR_ACTION_NEW_BEFORE:
		control->insertNewPage(control->getCurrentPageNo());
		break;
	case SIDEBAR_ACTION_NEW_AFTER:
		control->insertNewPage(control->getCurrentPageNo() + 1);
		break;
	default:
		break;
	case SIDEBAR_ACTION_NONE:
		break;
	}
}

void SidebarPreviewPages::updatePreviews()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	Document* doc = this->getControl()->getDocument();
	doc->lock();
	size_t len = doc->getPageCount();

	if (this->previews.size() == len)
	{
		doc->unlock();
		return;
	}

	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		delete p;
	}
	this->previews.clear();

	for (size_t i = 0; i < len; i++)
	{
		SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(i));
		this->previews.push_back(p);
		gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);
	}

	layout();
	doc->unlock();
}

void SidebarPreviewPages::pageSizeChanged(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page == size_t_npos || page >= this->previews.size())
	{
		return;
	}
	SidebarPreviewBaseEntry* p = this->previews[page];
	p->updateSize();
	p->repaint();

	layout();
}

void SidebarPreviewPages::pageChanged(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page == size_t_npos || page >= this->previews.size())
	{
		return;
	}

	SidebarPreviewBaseEntry* p = this->previews[page];
	p->repaint();
}

void SidebarPreviewPages::pageDeleted(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (page >= previews.size())
	{
		return;
	}

	delete previews[page];
	previews.erase(previews.begin() + page);

	// Unselect page, to prevent double selection displaying
	unselectPage();

	layout();
}

void SidebarPreviewPages::pageInserted(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	Document* doc = control->getDocument();
	doc->lock();

	SidebarPreviewBaseEntry* p = new SidebarPreviewPageEntry(this, doc->getPage(page));

	doc->unlock();

	this->previews.insert(this->previews.begin() + page, p);

	gtk_layout_put(GTK_LAYOUT(this->iconViewPreview), p->getWidget(), 0, 0);

	// Unselect page, to prevent double selection displaying
	unselectPage();

	layout();
}

/**
 * Unselect the last selected page, if any
 */
void SidebarPreviewPages::unselectPage()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	for (SidebarPreviewBaseEntry* p : this->previews)
	{
		p->setSelected(false);
	}
}

void SidebarPreviewPages::pageSelected(size_t page)
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		this->previews[this->selectedEntry]->setSelected(false);
	}
	this->selectedEntry = page;

	if (this->selectedEntry != size_t_npos && this->selectedEntry < this->previews.size())
	{
		SidebarPreviewBaseEntry* p = this->previews[this->selectedEntry];
		p->setSelected(true);
		scrollToPreview(this);

		int actions = 0;
		if (page != 0 && this->previews.size() != 0)
		{
			actions |= SIDEBAR_ACTION_MOVE_UP;
		}

		if (page != this->previews.size() - 1 && this->previews.size() != 0)
		{
			actions |= SIDEBAR_ACTION_MOVE_DOWN;
		}

		if (this->previews.size() != 0)
		{
			actions |= SIDEBAR_ACTION_COPY;
		}

		if (this->previews.size() > 1)
		{
			actions |= SIDEBAR_ACTION_DELETE;
		}

		this->toolbar->setHidden(false);
		this->toolbar->setButtonEnabled((SidebarActions)actions);
	}
}

void SidebarPreviewPages::openPreviewContextMenu()
{
	XOJ_CHECK_TYPE(SidebarPreviewPages);

	gtk_menu_popup(GTK_MENU(this->contextMenu), nullptr, nullptr, nullptr, nullptr, 3, gtk_get_current_event_time());
}

