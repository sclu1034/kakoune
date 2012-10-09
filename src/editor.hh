#ifndef editor_hh_INCLUDED
#define editor_hh_INCLUDED

#include "buffer.hh"
#include "selection.hh"
#include "filter.hh"
#include "idvaluemap.hh"
#include "memoryview.hh"
#include "filter_group.hh"

namespace Kakoune
{

class Register;

enum class SelectMode
{
    Replace,
    Extend,
    Append,
};

enum class InsertMode : unsigned
{
    Insert,
    Append,
    Replace,
    InsertAtLineBegin,
    AppendAtLineEnd,
    OpenLineBelow,
    OpenLineAbove
};

// An Editor is a buffer mutator
//
// The Editor class provides methods to manipulate a set of selections
// and to use these selections to mutate it's buffer.
class Editor : public SafeCountable
{
public:
    typedef std::function<SelectionAndCaptures (const Selection&)> Selector;
    typedef std::function<SelectionAndCapturesList (const Selection&)>  MultiSelector;


    Editor(Buffer& buffer);
    virtual ~Editor() {}

    Buffer& buffer() const { return m_buffer; }

    void erase();

    void insert(const String& string,
                InsertMode mode = InsertMode::Insert);
    void insert(const memoryview<String>& strings,
                InsertMode mode = InsertMode::Insert);

    void move_selections(LineCount move,
                         SelectMode mode = SelectMode::Replace);
    void move_selections(CharCount move,
                         SelectMode mode = SelectMode::Replace);
    void clear_selections();
    void keep_selection(int index);
    void remove_selection(int index);
    void select(const BufferIterator& iterator);
    void select(const Selector& selector,
                SelectMode mode = SelectMode::Replace);
    void select(SelectionAndCapturesList selections);
    void multi_select(const MultiSelector& selector);

    const SelectionAndCapturesList& selections() const { return m_selections; }
    std::vector<String>  selections_content() const;

    bool undo();
    bool redo();

    FilterGroup& filters() { return m_filters; }

    CandidateList complete_filterid(const String& prefix,
                                    size_t cursor_pos = String::npos);

    bool is_editing() const { return m_edition_level!= 0; }

private:
    friend class scoped_edition;
    void begin_edition();
    void end_edition();

    int m_edition_level;

    void check_invariant() const;

    friend class IncrementalInserter;
    virtual void on_incremental_insertion_begin() {}
    virtual void on_incremental_insertion_end() {}

    Buffer&       m_buffer;
    SelectionAndCapturesList m_selections;
    FilterGroup   m_filters;
};

struct scoped_edition
{
    scoped_edition(Editor& editor)
        : m_editor(editor)
    { m_editor.begin_edition(); }

    ~scoped_edition()
    { m_editor.end_edition(); }
private:
    Editor& m_editor;
};

// An IncrementalInserter manage insert mode
class IncrementalInserter
{
public:
    IncrementalInserter(Editor& editor, InsertMode mode = InsertMode::Insert);
    ~IncrementalInserter();

    void insert(const String& string);
    void insert(const memoryview<String>& strings);
    void erase();
    void move_cursors(const BufferCoord& offset);

    Buffer& buffer() const { return m_editor.buffer(); }
    Editor& editor() const { return m_editor; }

private:
    InsertMode     m_mode;
    Editor&        m_editor;
    scoped_edition m_edition;
};

}

#endif // editor_hh_INCLUDED

